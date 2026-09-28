#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    pid_t pid = fork();

    if (pid < 0) /* errror */
    {
        perror("fork");
        exit(-1);
    }
    else if (pid == 0) /* child process */
    {
        printf("child process:\n");
        printf("PID: %d\n", getpid());
        printf("PPID: %d\n", getppid());
        printf("PGID: %d\n", getpgrp());
        printf("SID: %d\n", getsid(0));
        printf("---------------------------\n");
        exit(1);
    }
    else /* parrent process */
    {
        printf("parrent process:\n");
        printf("PID: %d\n", getpid());
        printf("PPID: %d\n", getppid());
        printf("PGID: %d\n", getpgrp());
        printf("SID: %d\n", getsid(0));
        printf("---------------------------\n");
    }

    getchar();
    return 0;
}