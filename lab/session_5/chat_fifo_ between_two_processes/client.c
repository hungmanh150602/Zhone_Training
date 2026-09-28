/*
This example will reproduce communication between 2 processes:
The reader:
    read data from fifo named fifo_A_to_B
    write data to fifo named fifo_B_to_A
*/
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

int fd_read;
int fd_send;

/* function handle signal Ctrl + C */
void handle(int sig)
{
    /* close fd */
    close(fd_read);
    close(fd_send);

    exit(123);
}

/*
function read the message entered via the keyboard then send to server
*/
void *read_input(void *arg)
{
    char msg[512]; /* data to send */

    /* the loop comunication */
    while (1)
    {
        /* get input message from keyboard */
        if (fgets(msg, sizeof(msg), stdin) == NULL)
        {
            printf("ERROR: get input from keyboard.\n");
            return NULL;
        }
        msg[strcspn(msg, "\n")] = '\0';

        write(fd_send, msg, strlen(msg));
    }
    return NULL;
}

/*
function receive message sent from client
*/
void *rev_message(void *arg)
{
    char buffer[512]; /* buffer save data received */

    while (1)
    {
        int n = read(fd_read, buffer, sizeof(buffer) - 1);

        buffer[n] = '\0';

        printf("%s\n", buffer);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, handle);

    /* open fifo */
    fd_read = open("/tmp/fifo_A_to_B", O_RDONLY);
    fd_send = open("/tmp/fifo_B_to_A", O_WRONLY);

    pthread_t thread1, thread2;

    /* create 2 threads to hanlde input keyboard and receive message */
    if (pthread_create(&thread1, NULL, read_input, NULL) != 0)
    {
        printf("Error create thread 1.\n");
        return -1;
    }

    if (pthread_create(&thread2, NULL, rev_message, NULL) != 0)
    {
        printf("Error create thread 2.\n");
        return -1;
    }

    printf("client started.\n");

    /* join thread */
    if (pthread_join(thread1, NULL))
    {
        printf("Error join thread 1.\n");
        return -1;
    }

    if (pthread_join(thread2, NULL))
    {
        printf("Error join thread 2.\n");
        return -1;
    }

    return 0;
}