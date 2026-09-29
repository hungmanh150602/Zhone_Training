/*
CASE 0 : TCP
CASE 1 : UDP
CASE 3 : chat room
*/

#define CASE 3

#if CASE == 0

#include <sys/socket.h>
#include <arpa/inet.h> /* socket structure */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

int listen_fd;
int client_fd;

char sock_rev[128];
char buf[20];
char messg_send[128];
char *buff = NULL;

int init_ipv4_socket(const char *ip, const uint16_t port, const int sock_type)
{
    int fd_socket;
    struct sockaddr_in addr;

    /* create socket */
    fd_socket = socket(AF_INET, sock_type, 0);

    if (fd_socket == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    printf("socket fd = %d\n", fd_socket);

    /* Prepare IP address */
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;

    /* Host byte order -> Network byte order */
    addr.sin_port = htons(port);

    /* Presentation IP -> Binary IP */
    if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1)
    {
        perror("inet_pton");
        close(fd_socket);
        exit(EXIT_FAILURE);
    }

    /* reuse the address within TIME WAIT */
    int opt = 1;

    if (setsockopt(fd_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt");
        close(fd_socket);
        exit(EXIT_FAILURE);
    }

    /* Bind socket to ip:port */
    if (bind(fd_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("bind");
        close(fd_socket);
        exit(EXIT_FAILURE);
    }

    printf("bind successful\n");

    return fd_socket;
}

void getinfomation(int fd)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    char ip[INET_ADDRSTRLEN];

    getsockname(fd, (struct sockaddr *)&addr, &len);

    /* Binary IP -> Presentation IP */
    inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));

    printf("IP   = %s\n", ip);
    printf("Port = %u\n", ntohs(addr.sin_port));
    return;
}

void *socket_rev(void *arg)
{
    struct sockaddr_in peer;
    socklen_t len_peer = sizeof(peer);
    char ip[INET_ADDRSTRLEN];
    uint16_t port;

    getpeername(client_fd, (struct sockaddr *)&peer, &len_peer);

    inet_ntop(AF_INET, &peer.sin_addr, ip, sizeof(ip));
    port = ntohs(peer.sin_port);

    while (1)
    {
        int n = read(client_fd, sock_rev, sizeof(sock_rev) - 1);

        if (n == 0)
        {
            printf("Client port[%d] closed!\n", port);
            kill(getpid(), SIGUSR1);
            sleep(1);
            continue;
        }

        if (n < 0)
        {
            perror("read");
            // kill(getpid(), SIGUSR1);
            sleep(1);
            continue;
        }

        sock_rev[n] = '\0';

        printf("Client port[%d] sent: %s\n", port, sock_rev);

        /* feedback the client */
        n = write(client_fd, sock_rev, strlen(sock_rev));

        if (n < 0)
        {
            perror("write");
            // kill(getpid(), SIGUSR1);
            sleep(1);
            continue;
        }
    }

    return NULL;
}

void *getinput(void *arg)
{
    while (1)
    {
        if (fgets(messg_send, sizeof(messg_send), stdin) == NULL)
        {
            printf("error fgets\n");
            return NULL;
        }
        messg_send[strcspn(messg_send, "\n")] = '\0';

        // strcpy(messg_send, "GET OUT NOW >.<' !!!");

        int n = write(client_fd, messg_send, strlen(messg_send));

        if (n < 0)
        {
            perror("write");
            // kill(getpid(), SIGUSR1);
            // break;
            sleep(1);
            continue;
        }

        sleep(1);
    }

    return NULL;
}

void handle(int sig)
{
    switch (sig)
    {
    case SIGINT:
        close(listen_fd);
        close(client_fd);
        exit(0);
        break;

    case SIGUSR1:
        close(client_fd);
        buff = "\nchild exit.\n";
        write(STDOUT_FILENO, buff, strlen(buff));
        _exit(1);
        break;

    case SIGPIPE:
        buff = "client don't read.\n";
        write(STDOUT_FILENO, buff, strlen(buff));
        break;

    case SIGCHLD:
        while (waitpid(-1, NULL, WNOHANG) > 0)
        {
        }
        break;

    default:
        break;
    }
}

int main(int argc, char *argv[])
{
    signal(SIGINT, handle);
    signal(SIGCHLD, handle);

    if (argc != 3)
    {
        printf("the format is: \"%s ip port\" to create socket.\n", argv[0]);
        exit(0);
    }
    /* create and blind socket object */
    listen_fd = init_ipv4_socket(argv[1], (uint16_t)atoi(argv[2]), SOCK_STREAM);

    getinfomation(listen_fd); /* Ask kernel: What address is this socket actually using? */

    /* listen the client to connect */
    if (listen(listen_fd, 100) == -1)
    {
        printf("error listen");
        return -1;
    }

    /* accept client */
    struct sockaddr_in client;
    socklen_t len_client;

    while (1)
    {
        len_client = sizeof(client);
        client_fd = accept(listen_fd, (struct sockaddr *)&client, &len_client);

        if (client_fd == -1)
        {
            perror("accept");
            continue;
        }

        printf("Client connected!\n");

        if (fork() == 0) /* child process the request */
        {
            close(listen_fd); /* child close its listen fd */

            /* BEGIN CODE */
            signal(SIGUSR1, handle);
            signal(SIGPIPE, handle);

            /* get infomation of client */
            printf("client ip: %s\n", inet_ntop(AF_INET, &client.sin_addr, buf, sizeof(buf)));
            printf("client port: %d\n", ntohs(client.sin_port));

            /* NON Blocking */
            int flag = fcntl(client_fd, F_GETFL, 0);
            fcntl(client_fd, F_SETFL, flag | O_NONBLOCK);

            /* create two threads to process request */
            pthread_t thread1, thread2;

            pthread_create(&thread1, NULL, socket_rev, NULL); /* receive message */
            pthread_create(&thread2, NULL, getinput, NULL);   /* send message */

            pthread_join(thread1, NULL);
            pthread_join(thread2, NULL);

            /* END OF CODE */

            close(client_fd); /* child close client fd when done */
            _exit(0);         /* child exit */
        }

        /* parent close socket */
        close(client_fd);
    }

    close(listen_fd);
    return 0;
}

#elif CASE == 1

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h> /* socket structure */
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("the format is: \"%s ip port\" to create socket.\n", argv[0]);
        exit(0);
    }

    int udp_server = socket(AF_INET, SOCK_DGRAM, 0);

    if (udp_server == -1)
    {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr_server;

    memset(&addr_server, 0, sizeof(addr_server));
    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &addr_server.sin_addr) != 1)
    {
        perror("inet_pton");
        return -1;
    }

    if (bind(udp_server, (struct sockaddr *)&addr_server, sizeof(addr_server)) == -1)
    {
        perror("bind");
        close(udp_server);
        return -1;
    }

    while (1)
    {
        struct sockaddr_in addr_rev;
        socklen_t rev_len = sizeof(addr_rev);
        char rev_msg[1024];

        int n = recvfrom(udp_server, rev_msg, sizeof(rev_msg) - 1, 0,
                         (struct sockaddr *)&addr_rev, &rev_len);

        if (n < 0)
        {
            perror("recvfrom");
            continue;
        }

        rev_msg[n] = '\0';

        printf("Received: %s\n", rev_msg);

        int m = sendto(udp_server, rev_msg, n, 0, (struct sockaddr *)&addr_rev, rev_len);

        if (m < 0)
        {
            perror("sendto");
            continue;
        }
    }

    close(udp_server);
    return 0;
}


#elif CASE == 3

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h> /* socket structure */
#include <sys/select.h>
#include <string.h>
#include <unistd.h>

static int send_all(int fd, const char *data, size_t length)
{
    size_t sent = 0;

    while (sent < length)
    {
        ssize_t n = send(fd, data + sent, length - sent, MSG_NOSIGNAL);

        if (n < 0 && errno == EINTR)
        {
            continue;
        }
        if (n <= 0)
        {
            return -1;
        }
        sent += (size_t)n;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <IPv4 address> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *port_end;
    errno = 0;
    unsigned long port = strtoul(argv[2], &port_end, 10);
    if (errno != 0 || *argv[2] == '\0' || *port_end != '\0' || port > 65535)
    {
        fprintf(stderr, "Invalid port: %s\n", argv[2]);
        return EXIT_FAILURE;
    }

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    int reuse_addr = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr,
                   sizeof(reuse_addr)) < 0)
    {
        perror("setsockopt");
        close(listen_fd);
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) != 1)
    {
        fprintf(stderr, "Invalid IPv4 address: %s\n", argv[1]);
        close(listen_fd);
        return EXIT_FAILURE;
    }

    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(listen_fd);
        return EXIT_FAILURE;
    }
    if (listen(listen_fd, SOMAXCONN) < 0)
    {
        perror("listen");
        close(listen_fd);
        return EXIT_FAILURE;
    }
    if (listen_fd >= FD_SETSIZE)
    {
        fprintf(stderr, "Listening socket exceeds FD_SETSIZE\n");
        close(listen_fd);
        return EXIT_FAILURE;
    }

    fd_set master_set;
    FD_ZERO(&master_set);
    FD_SET(listen_fd, &master_set);
    int max_fd = listen_fd;
    printf("Chat room listening on %s:%lu\n", argv[1], port);

    for (;;)
    {
        fd_set read_set = master_set;
        int ready = select(max_fd + 1, &read_set, NULL, NULL, NULL);

        if (ready < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            perror("select");
            break;
        }

        if (FD_ISSET(listen_fd, &read_set))
        {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr,
                                   &client_len);

            if (client_fd < 0)
            {
                if (errno != EINTR)
                {
                    perror("accept");
                }
            }
            else if (client_fd >= FD_SETSIZE)
            {
                fprintf(stderr, "Rejecting client: descriptor limit reached\n");
                close(client_fd);
            }
            else
            {
                FD_SET(client_fd, &master_set);
                if (client_fd > max_fd)
                {
                    max_fd = client_fd;
                }

                char client_ip[INET_ADDRSTRLEN];
                if (inet_ntop(AF_INET, &client_addr.sin_addr, client_ip,
                              sizeof(client_ip)) == NULL)
                {
                    strcpy(client_ip, "unknown");
                }
                printf("Client connected: %s:%u (fd=%d)\n", client_ip,
                       ntohs(client_addr.sin_port), client_fd);
            }

            if (--ready == 0)
            {
                continue;
            }
        }

        for (int client_fd = 0; client_fd <= max_fd && ready > 0; ++client_fd)
        {
            if (client_fd == listen_fd || !FD_ISSET(client_fd, &master_set) ||
                !FD_ISSET(client_fd, &read_set))
            {
                continue;
            }

            --ready;
            char message[1024];
            ssize_t n = recv(client_fd, message, sizeof(message), 0);

            if (n <= 0)
            {
                if (n < 0 && errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
                {
                    perror("recv");
                }
                if (n == 0 || (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK))
                {
                    printf("Client disconnected (fd=%d)\n", client_fd);
                    close(client_fd);
                    FD_CLR(client_fd, &master_set);
                    if (client_fd == max_fd)
                    {
                        while (max_fd > listen_fd && !FD_ISSET(max_fd, &master_set))
                        {
                            --max_fd;
                        }
                    }
                }
                continue;
            }

            for (int peer_fd = 0; peer_fd <= max_fd; ++peer_fd)
            {
                if (peer_fd == listen_fd || peer_fd == client_fd ||
                    !FD_ISSET(peer_fd, &master_set))
                {
                    continue;
                }

                if (send_all(peer_fd, message, (size_t)n) < 0)
                {
                    printf("Removing unreachable client (fd=%d)\n", peer_fd);
                    close(peer_fd);
                    FD_CLR(peer_fd, &master_set);
                    if (peer_fd == max_fd)
                    {
                        while (max_fd > listen_fd && !FD_ISSET(max_fd, &master_set))
                        {
                            --max_fd;
                        }
                    }
                }
            }
        }
    }

    for (int fd = 0; fd <= max_fd; ++fd)
    {
        if (FD_ISSET(fd, &master_set))
        {
            close(fd);
        }
    }
    return EXIT_FAILURE;
}
#endif