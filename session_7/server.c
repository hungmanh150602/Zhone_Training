/*
CASE 0 : TCP
CASE 1 : UDP
CASE 3 : chat room select
CASE 4 : chat room poll
*/

#define CASE 4

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
#include <signal.h>
#include <bits/sigaction.h>

#define MAX_CLIENTS 100

int listen_fd;

int init_ipv4_socket(const char *ip, const uint16_t port, const int sock_type)
{
    struct sockaddr_in addr;

    /* create socket */
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (listen_fd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    printf("socket fd = %d\n", listen_fd);

    /* Prepare IP address */
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    /* Host byte order -> Network byte order */
    addr.sin_port = htons(port);
    /* Presentation IP -> Binary IP */
    if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1)
    {
        perror("inet_pton");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    /* reuse the address within TIME WAIT */
    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    /* Bind socket to ip:port */
    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("bind");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    printf("bind successful\n");
    return listen_fd;
}

void handle(int sig)
{
    switch (sig)
    {
    case SIGINT:
        close(listen_fd);
        exit(0);
        break;

    default:
        break;
    }
}

int main(int argc, char *argv[])
{
    struct sigaction sa;
    sa.sa_handler = handle;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <IPv4 address> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* create listen socket */
    listen_fd = init_ipv4_socket(argv[1], (uint16_t)atoi(argv[2]), SOCK_STREAM);

    /* listen for incoming connections */
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

    /* initialize the fd_set for select */
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(listen_fd, &readfds);
    int max_fd = listen_fd;

    while (1)
    {
        fd_set temp_fds = readfds; /* copy the fd_set for select */
        int client_fd = select(max_fd + 1, &temp_fds, NULL, NULL, NULL);
        if (client_fd < 0)
        {
            if (errno == EINTR)
                continue; /* interrupted by signal, retry */
            perror("select");
            break;
        }

        /* accept and set new client to the fd_set */
        if (FD_ISSET(listen_fd, &temp_fds))
        {
            /* accept new client connection */
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int new_client = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
            if (new_client < 0)
            {
                perror("accept");
                continue;
            }

            /* add new client socket to the fd_set */
            FD_SET(new_client, &readfds);
            if (new_client > max_fd)
                max_fd = new_client;

            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
            printf("New connection from %s:%d\n", client_ip, ntohs(client_addr.sin_port));
        }

        /* check for data from existing clients */
        for (int fd = 0; fd <= max_fd; fd++)
        {
            if (fd != listen_fd && FD_ISSET(fd, &temp_fds))
            {
                char buffer[1024];
                ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
                if (bytes_read <= 0)
                {
                    if (bytes_read == 0)
                    {
                        printf("Client on fd %d disconnected\n", fd);
                    }
                    else
                    {
                        perror("read");
                    }
                    close(fd);
                    FD_CLR(fd, &readfds);
                }
                else
                {
                    buffer[bytes_read] = '\0';
                    printf("Received from fd %d: %s\n", fd, buffer);
                    /* Echo back the received data */
                    write(fd, buffer, bytes_read);
                }
            }
        }
    }

    close(listen_fd);
    return EXIT_FAILURE;
}

#elif CASE == 4

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <signal.h>
#include <bits/sigaction.h>

#define MAX_CLIENTS 100

int listen_fd;

int init_ipv4_socket(const char *ip, const uint16_t port, const int sock_type)
{
    struct sockaddr_in addr;

    /* create socket */
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (listen_fd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    printf("socket fd = %d\n", listen_fd);

    /* Prepare IP address */
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    /* Host byte order -> Network byte order */
    addr.sin_port = htons(port);
    /* Presentation IP -> Binary IP */
    if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1)
    {
        perror("inet_pton");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    /* reuse the address within TIME WAIT */
    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    /* Bind socket to ip:port */
    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("bind");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    printf("bind successful\n");
    return listen_fd;
}

void handle(int sig)
{
    switch (sig)
    {
    case SIGINT:
        close(listen_fd);
        exit(0);
        break;

    default:
        break;
    }
}

int main(int argc, char *argv[])
{
    struct sigaction sa;
    sa.sa_handler = handle;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <IPv4 address> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    listen_fd = init_ipv4_socket(argv[1], (uint16_t)atoi(argv[2]), SOCK_STREAM);

    /* listen for incoming connections */
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

    /* Initialize pollfd array */
    struct pollfd fds[MAX_CLIENTS];
    
    fds[0].fd = listen_fd;
    fds[0].events = POLLIN;
    for (int i = 1; i < MAX_CLIENTS; i++)
    {
        fds[i].fd = -1; // Initialize all other fds to -1
    }

    while (1)
    {
        int ret = poll(fds, MAX_CLIENTS, -1);
        if (ret < 0)
        {
            perror("poll");
            break;
        }

        /* new connection */
        if (fds[0].revents & POLLIN)
        {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int new_client = accept(listen_fd,
                                    (struct sockaddr *)&client_addr,
                                    &client_len);
            if (new_client < 0)
            {
                perror("accept");
                continue;
            }

            printf("New connection from %s:%d\n",
                   inet_ntoa(client_addr.sin_addr),
                   ntohs(client_addr.sin_port));

            /* Add new client to pollfd array */
            for (int i = 1; i < MAX_CLIENTS; i++)
            {
                if (fds[i].fd == -1)
                {
                    fds[i].fd = new_client;
                    fds[i].events = POLLIN;
                    break;
                }
            }
        }

        /* check for data from existing clients */
        for (int i = 1; i < MAX_CLIENTS; i++)
        {
            if (fds[i].fd != -1 && fds[i].revents & POLLIN)
            {
                char buffer[1024];
                ssize_t bytes_read = read(fds[i].fd, buffer, sizeof(buffer) - 1);
                if (bytes_read <= 0)
                {
                    if (bytes_read == 0)
                    {
                        printf("Client on fd %d disconnected\n", fds[i].fd);
                    }
                    else
                    {
                        perror("read");
                    }
                    close(fds[i].fd);
                    fds[i].fd = -1; /* Remove client from pollfd array */
                }
                else
                {
                    buffer[bytes_read] = '\0';
                    printf("Received from fd %d: %s\n", fds[i].fd, buffer);
                    /* Echo back the received data */
                    write(fds[i].fd, buffer, bytes_read);
                }
            }
        }
    }

    close(listen_fd);
    return EXIT_SUCCESS;
}
#endif