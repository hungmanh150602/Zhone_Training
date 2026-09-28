#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    pid_t pid = fork();

    /* child will sleep and wait for parrent to die first */
    if (pid == 0)
    {
        sleep(10);

        /* print out PID and PPID of child process */
        printf("Child PID  = %d\n", getpid());
        printf("Child PPID = %d\n", getppid());

        return 0;
    }
    else
    {
        /* print out PID and PPID of parent process */
        printf("Parent PID  = %d\n", getpid());
        printf("Child PID = %d\n", pid);
    }

    printf("Parent exiting...\n");

    exit(0);
}