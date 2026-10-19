#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <bits/sigaction.h>
#include <string.h>

void handle(int sig)
{
    char buff[30];
    sprintf(buff, "\nterminate by signal %d\n", sig);
    write(STDOUT_FILENO, buff, strlen(buff));
    _exit(0);
}

int main(int argc, char *argv[])
{
    struct sigaction sa;
    sa.sa_handler = handle;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    int led = open("/dev/gpio_demo", O_RDWR);

    if (led == -1)
    {
        perror("open");
        return -1;
    }

    while (1)
    {
        int m = write(led, "1", 1);
        if (m == -1)
        {
            perror("write");
            return -1;
        }
        sleep(1);

        m = write(led, "0", 1);
        if (m == -1)
        {
            perror("write");
            return -1;
        }
        sleep(1);
    }
    return 0;
}