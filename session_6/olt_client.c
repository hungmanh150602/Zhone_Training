#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(int argc, char **argv)
{
    const char *server_ip =
        argc > 1
            ? argv[1]
            : "127.0.0.1";

    int port =
        argc > 2
            ? atoi(argv[2])
            : 9000;

    /* --------------------------------------------------------
     * socket()
     * -------------------------------------------------------- */

    int fd =
        socket(
            AF_INET,
            SOCK_STREAM,
            0);

    if (fd < 0)
    {
        perror("socket");
        return 1;
    }

    /* --------------------------------------------------------
     * Server address
     * -------------------------------------------------------- */

    struct sockaddr_in server_addr;

    memset(
        &server_addr,
        0,
        sizeof(server_addr));

    server_addr.sin_family =
        AF_INET;

    server_addr.sin_port =
        htons(port);

    if (
        inet_pton(
            AF_INET,
            server_ip,
            &server_addr.sin_addr) != 1)
    {
        fprintf(
            stderr,
            "Invalid server IP\n");

        close(fd);
        return 1;
    }

    /* --------------------------------------------------------
     * connect()
     * -------------------------------------------------------- */

    if (
        connect(
            fd,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(fd);
        return 1;
    }

    printf(
        "Connected to OLT %s:%d\n",
        server_ip,
        port);

    printf(
        "Commands:\n"
        "  show\n"
        "  onu <id>\n"
        "  set <id> up\n"
        "  set <id> down\n"
        "  alarm <id>\n"
        "  config-alarm <id>\n"
        "  quit\n\n");

    char command[256];
    char response[BUFFER_SIZE];

    while (1)
    {
        printf("> ");
        fflush(stdout);

        if (
            fgets(
                command,
                sizeof(command),
                stdin) == NULL)
        {
            break;
        }

        if (
            send(
                fd,
                command,
                strlen(command),
                0) < 0)
        {
            perror("send");
            break;
        }

        if (
            strncmp(
                command,
                "quit",
                4) == 0)
        {
            break;
        }

        ssize_t n =
            recv(
                fd,
                response,
                sizeof(response) - 1,
                0);

        if (n <= 0)
        {
            printf(
                "Server disconnected\n");

            break;
        }

        response[n] = '\0';

        printf(
            "%s",
            response);
    }

    close(fd);

    return 0;
}