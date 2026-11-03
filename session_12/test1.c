#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

#include <sys/socket.h>
#include <netdb.h>

#define BUFFER_SIZE 4096
#define HEADER_SIZE 16384

struct http_response
{
    int status_code;
    size_t content_length;
    int has_content_length;
    char content_type[128];
};

static int send_all(int sockfd, const char *data, size_t len)
{
    size_t total = 0;

    while (total < len)
    {
        ssize_t sent = send(sockfd, data + total, len - total, MSG_NOSIGNAL);

        if (sent < 0)
        {
            if (errno == EINTR)
                continue;

            perror("send");
            return -1;
        }

        if (sent == 0)
            return -1;

        total += (size_t)sent;
    }

    return 0;
}

/* Find CRLF CRLF in HTTP header */
static int find_header_end(const char *buffer, size_t len)
{
    if (len < 4)
        return -1;

    for (size_t i = 0; i <= len - 4; i++)
    {
        if (memcmp(buffer + i, "\r\n\r\n", 4) == 0)
            return (int)(i + 4);
    }

    return -1;
}

/* Parse HTTP response headers */
static int parse_headers(char *header,
                         struct http_response *response)
{
    memset(response, 0, sizeof(*response));

    char *saveptr = NULL;
    char *line = strtok_r(header, "\r\n", &saveptr);

    if (line == NULL ||
        sscanf(line, "HTTP/%*u.%*u %d", &response->status_code) != 1)
    {
        fprintf(stderr, "Invalid HTTP status line\n");
        return -1;
    }

    while ((line = strtok_r(NULL, "\r\n", &saveptr)) != NULL)
    {

        if (strncasecmp(line, "Content-Length:", 15) == 0)
        {
            char *value = line + 15;
            char *endptr = NULL;

            while (*value == ' ' || *value == '\t')
                value++;

            errno = 0;
            unsigned long long length = strtoull(value, &endptr, 10);

            if (errno != 0 || endptr == value ||
                *endptr != '\0' || length > SIZE_MAX)
            {
                fprintf(stderr, "Invalid Content-Length\n");
                return -1;
            }

            response->content_length = (size_t)length;
            response->has_content_length = 1;
        }

        else if (strncasecmp(line, "Content-Type:", 13) == 0)
        {
            const char *value = line + 13;

            while (*value == ' ' || *value == '\t')
                value++;

            snprintf(response->content_type,
                     sizeof(response->content_type),
                     "%s",
                     value);
        }
    }

    return 0;
}

static int save_body(FILE *file,
                     const char *data,
                     size_t len,
                     size_t *total)
{
    if (len == 0)
        return 0;

    size_t written = fwrite(data, 1, len, file);

    if (written != len)
    {
        perror("fwrite");
        return -1;
    }

    *total += written;
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        fprintf(stderr,
                "Usage: %s <host> <port> <path> <output>\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    const char *host = argv[1];
    const char *port = argv[2];
    const char *path = argv[3];
    const char *output = argv[4];

    int sockfd = -1;
    int ret = EXIT_FAILURE;

    struct addrinfo hints;
    struct addrinfo *result = NULL;
    struct addrinfo *rp = NULL;

    FILE *file = NULL;

    char request[1024];
    char buffer[BUFFER_SIZE];
    char header[HEADER_SIZE];

    size_t header_len = 0;
    size_t body_received = 0;

    int header_complete = 0;

    struct http_response response = {0};

    /* 1. Resolve hostname */

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(host, port, &hints, &result);

    if (status != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n",
                gai_strerror(status));
        goto cleanup;
    }

    /* 2. Create TCP connection */

    for (rp = result; rp != NULL; rp = rp->ai_next)
    {

        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

        if (sockfd < 0)
            continue;

        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) == 0)
        {
            break;
        }

        close(sockfd);
        sockfd = -1;
    }

    if (sockfd < 0)
    {
        fprintf(stderr, "Cannot connect to server\n");
        goto cleanup;
    }

    printf("TCP connection established\n");

    /* 3. Construct HTTP GET request */

    int request_len = snprintf(request, sizeof(request),
                               "GET %s HTTP/1.1\r\n"
                               "Host: %s\r\n"
                               "Connection: close\r\n"
                               "\r\n",
                               path, host);

    if (request_len < 0 ||
        (size_t)request_len >= sizeof(request))
    {
        fprintf(stderr, "HTTP request too long\n");
        goto cleanup;
    }

    /* 4. Send request */

    if (send_all(sockfd, request, (size_t)request_len) < 0)
        goto cleanup;

    printf("HTTP GET request sent\n");

    /* 5. Open output file */

    file = fopen(output, "wb");

    if (file == NULL)
    {
        perror("fopen");
        goto cleanup;
    }

    /* 6. Receive and parse HTTP response */

    while (1)
    {

        ssize_t received = recv(sockfd, buffer, sizeof(buffer), 0);

        if (received < 0)
        {
            if (errno == EINTR)
                continue;

            perror("recv");
            goto cleanup;
        }

        if (received == 0)
            break;

        size_t offset = 0;
        size_t chunk_len = (size_t)received;

        /* 6.1. Receive HTTP headers */

        if (!header_complete)
        {

            while (offset < chunk_len && !header_complete)
            {

                if (header_len >= HEADER_SIZE - 1)
                {
                    fprintf(stderr, "HTTP header too large\n");
                    goto cleanup;
                }

                header[header_len++] = buffer[offset++];

                int end = find_header_end(header, header_len);

                if (end >= 0)
                {

                    header_complete = 1;

                    header_len = (size_t)end;
                    header[header_len] = '\0';

                    if (parse_headers(header, &response) < 0)
                        goto cleanup;

                    printf("\nHTTP Status: %d\n", response.status_code);

                    printf("Content-Type: %s\n",
                           response.content_type[0] ? response.content_type : "unknown");

                    if (response.has_content_length)
                    {
                        printf("Content-Length: %zu bytes\n", response.content_length);
                    }
                    else
                    {
                        printf("Content-Length: unknown\n");
                    }

                    printf("\nReceiving HTTP body...\n");
                }
            }
        }

        /* 6.2. Save HTTP body */

        if (header_complete && offset < chunk_len)
        {

            size_t body_len = chunk_len - offset;

            if (response.has_content_length)
            {

                size_t remaining = response.content_length - body_received;

                if (body_len > remaining)
                    body_len = remaining;
            }

            if (save_body(file,
                          buffer + offset,
                          body_len,
                          &body_received) < 0)
            {
                goto cleanup;
            }
        }

        if (header_complete &&
            response.has_content_length &&
            body_received == response.content_length)
        {
            break;
        }
    }

    if (!header_complete)
    {
        fprintf(stderr, "Incomplete HTTP response header\n");
        goto cleanup;
    }

    if (response.has_content_length &&
        body_received != response.content_length)
    {
        fprintf(stderr,
                "Incomplete body: expected %zu, received %zu\n",
                response.content_length,
                body_received);
        goto cleanup;
    }

    printf("\nHTTP response completed\n");
    printf("Body received: %zu bytes\n", body_received);
    printf("Saved to: %s\n", output);

    ret = response.status_code >= 200 &&
                  response.status_code < 300
              ? EXIT_SUCCESS
              : EXIT_FAILURE;

cleanup:

    if (file != NULL)
        fclose(file);

    if (sockfd >= 0)
        close(sockfd);

    if (result != NULL)
        freeaddrinfo(result);

    return ret;
}