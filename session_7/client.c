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
#include <signal.h>
#include <pthread.h>

int fd_client;

char sock_rev[128];
char buf[20];
char messg_send[128];

void *socket_rev(void *arg)
{
    while (1)
    {
        int n = read(fd_client, sock_rev, sizeof(sock_rev) - 1);

        if (n == 0)
        {
            printf("server closed!\n");
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

        printf("Server sent: %s\n", sock_rev);
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
            kill(getpid(), SIGUSR1);
            break;
        }
        messg_send[strcspn(messg_send, "\n")] = '\0';

        int n = write(fd_client, messg_send, strlen(messg_send));

        if (n < 0)
        {
            perror("write");
            // kill(getpid(), SIGUSR1);
            // break;
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
        close(fd_client);
        exit(0);
        break;

    case SIGUSR1:
        close(fd_client);
        printf("\nclient closed.\n");
        exit(1);
        break;

    case SIGPIPE:
        printf("server don't read.\n");
        break;

    default:
        break;
    }
}

int main(int argc, char *argv[])
{
    signal(SIGINT, handle);
    signal(SIGUSR1, handle);
    signal(SIGPIPE, handle);

    if (argc != 3)
    {
        printf("to connect, the format is: \"%s ip port\"\n", argv[0]);
        exit(0);
    }

    /* create and socket object */
    struct sockaddr_in client;
    socklen_t len_client;

    fd_client = socket(AF_INET, SOCK_STREAM, 0);

    memset(&client, 0, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &client.sin_addr) != 1)
    {
        perror("inet_pton");
        close(fd_client);
        exit(EXIT_FAILURE);
    }

    /* connect to server */
    len_client = sizeof(client);
    if (connect(fd_client, (struct sockaddr *)&client, len_client) == -1)
    {
        perror("connect");
        return -1;
    }

    printf("Connected to:\n\
        ip: %s\n\
        port: %s\n",
           argv[1], argv[2]);

    // shutdown(fd_client, SHUT_RD);

    pthread_t thread1, thread2;

    pthread_create(&thread1, NULL, socket_rev, &argv[2]);
    pthread_create(&thread2, NULL, getinput, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    close(fd_client);
    getchar();

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

    int udp_client = socket(AF_INET, SOCK_DGRAM, 0);

    if (udp_client == -1)
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

    while (1)
    {
        char msg_send[1024];

        printf("Input: ");

        if (fgets(msg_send, sizeof(msg_send), stdin) == NULL)
            break;

        int m = sendto(udp_client, msg_send, strlen(msg_send), 0,
                       (struct sockaddr *)&addr_server, sizeof(addr_server));

        if (m < 0)
        {
            perror("sendto");
            continue;
        }

        struct sockaddr_in addr_rev;
        socklen_t addr_len = sizeof(addr_rev);

        int n = recvfrom(udp_client, msg_send, sizeof(msg_send) - 1, 0,
                         (struct sockaddr *)&addr_rev, &addr_len);

        if (n < 0)
        {
            perror("recvfrom");
            continue;
        }

        msg_send[n] = '\0';

        printf("Echo: %s\n", msg_send);
    }

    close(udp_client);
    return 0;
}

#endif