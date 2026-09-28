/*
CASE 0 : TCP
CASE 1 : UDP
*/

#define CASE 1

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

int listen_fd;
int client_fd;

char sock_rev[128];
char buf[20];
char messg_send[128];

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
            break;
        }

        if (n < 0)
        {
            perror("read");
            kill(getpid(), SIGUSR1);
            break;
        }

        sock_rev[n] = '\0';

        printf("Client port[%d] sent: %s\n", port, sock_rev);

        /* feedback the client */
        n = write(client_fd, sock_rev, strlen(sock_rev));

        if (n < 0)
        {
            perror("write");
            kill(getpid(), SIGUSR1);
            break;
        }
    }

    return NULL;
}

void *getinput(void *arg)
{
    while (1)
    {
        // if (fgets(messg_send, sizeof(messg_send), stdin) == NULL)
        // {
        //     printf("error fgets\n");
        //     return NULL;
        // }
        // messg_send[strcspn(messg_send, "\n")] = '\0';

        strcpy(messg_send, "GET OUT NOW >.<' !!!");

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
        printf("\nchild exit.\n");
        _exit(1);
        break;

    case SIGPIPE:
        printf("client don't read.\n");
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

        if(m < 0)
        {
            perror("sendto");
            continue;
        }

    }

    close(udp_server);
    return 0;
}

#endif