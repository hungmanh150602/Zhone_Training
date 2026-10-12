/*
CASE 0 : test
CASE 1 : multiple child
CASE 2 : vfork
CASE 3 :
CASE 4 : zombie
CASE 5 : orphan
CASE 6 : exec
CASE 7 : process group
CASE 8 : session
CASE 9 : Job control
CASE 10 : Race condition betwen fork() and exec()
CASE 11 : Daemon
CASE 12 : race condition
*/

#define CASE 11

#if CASE == 0
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    printf("Hello\n");

    fork();

    return 0;
}

#elif CASE == 1
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(void)
{
    int x = 100;

    printf("Before fork\n");
    printf("x = %d\n", x);
    printf("PID = %d\n", getpid());   /* pid of current process */
    printf("PPID = %d\n", getppid()); /* pid of parent process */
    printf("---------------------------------------------------\n");

    pid_t pid = fork();

    printf("After fork child\n");

    if (pid < 0) /* error */
    {
        perror("fork");
        return 1;
    }

    if (pid == 0) /* child process */
    {
        pid_t pid = fork();

        printf("After fork grandchildren\n");

        if (pid < 0) /* error*/
        {
            perror("fork");
            return 1;
        }

        if (pid == 0) /* grandchildren process*/
        {
            x = 1000;
            printf("x = %d\n", x);
            printf("Grandchildren: PID = %d\n", getpid());
            printf("Grandchildren: PPID = %d\n", getppid());
            printf("---------------------------------------------------\n");
        }
        else /* child process*/
        {
            x = 200;
            printf("x = %d\n", x);
            printf("Child:  PID = %d\n", getpid());
            printf("Child: PPID = %d\n", getppid());
            printf("Child: grandchildren PID = %d\n", pid);
            // printf("---------------------------------------------------\n");

            // kill(getpid());
            // printf("---------------------------------------------------\n");
            // printf("After kill\n");
            // printf("PID = %d\n", getpid());   /* pid of current process */
            // printf("PPID = %d\n", getppid()); /* pid of parent process */
            printf("---------------------------------------------------\n");
        }
    }
    else /* parrent process */
    {
        x = 300;
        printf("x = %d\n", x);
        printf("Parent: PID = %d\n", getpid());
        printf("Parent: child PID = %d\n", pid);
        printf("---------------------------------------------------\n");
        // wait(NULL);
        exit(0);
    }
    // printf("Hello!\n");
    // printf("%d\n", x);
    // printf("---------------------------------------------------\n");

    getchar();
    return 0;
}
#elif CASE == 2
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(void)
{
    int x = 20;

    printf("hello\n"); /* print "hello" before vfork()*/

    pid_t pid = vfork(); /* create child 1 in parent process */

    if (pid == 0) /* in child 1 process */
    {
        printf("child1 x = %d\n", x);

        pid_t pid1 = vfork(); /* create child 2 in child 1 process */

        if (pid1 == 0) /* in child 2 process */
        {
            printf("child2\n");
            // exit(12); /* i comment it */
        }
        else /* in child 1 process */
        {
            // exit(0); /* i comment it */
        }
    }
    else /* in parent process */
    {
        // x = 40;
        printf("Parent : %d\n", x);
    }

    printf("parrent + child : x = %d\n", x);

    return 24;
}
#elif CASE == 3
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid == 0)
    {
        /* Child */

        printf("Child:\n");
        printf("    PID  = %d\n", getpid());
        printf("    PPID = %d\n", getppid());

        printf("Child: executing ls...\n");

        execlp("ls", "ls", "-l", NULL);

        /* Only reached if exec fails */
        perror("execlp");
        _exit(1);
    }
    else
    {
        /* Parent */

        printf("Parent:\n");
        printf("    PID       = %d\n", getpid());
        printf("    Child PID = %d\n", pid);

        int status;

        waitpid(pid, &status, 0);

        if (WIFEXITED(status))
        {
            printf("Child exited normally\n");
            printf("Exit status = %d\n", WEXITSTATUS(status));
        }
    }

    return 0;
}
#elif CASE == 4
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    printf("Parent PID = %d\n", getpid());

    for (int i = 0; i < 10000; i++)
    {

        pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork");
            break;
        }

        if (pid == 0)
        {
            // Child exit immediately
            exit(0);
        }

        printf("Created child %d PID = %d\n", i + 1, pid);
    }

    printf("Parent sleeping...\n");
    getchar();

    return 0;
}
#elif CASE == 5
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    pid_t pid = fork();

    if (pid == 0)
    {
        printf("Child:\n------------------------------\n");
        /* child before parrent die */
        printf("Before parrent die:\n--------\n");
        printf("Child PID  = %d\n", getpid());
        printf("Child PPID = %d\n", getppid());

        /* child will sleep 10 seconds to wait parrent die */
        getchar();

        printf("After parrent die:\n--------\n");
        printf("Child PID  = %d\n", getpid());
        printf("Child PPID = %d\n", getppid());

        return 0;
    }
    else
    {
        printf("Parrent:\n------------------------------\n");
        printf("Parent PID  = %d\n", getpid());
        printf("Child PID = %d\n", pid);
    }

    getchar();

    printf("Parent exiting...\n");

    exit(0);
}
#elif CASE == 6
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }
    if (pid == 0)
    {
        /* code */
        printf("Before exec\n");
        printf("PIP: %d\n", getpid());
        printf("PPID: %d\n", getppid());

        char *arg[] =
            {
                "",
                NULL};

        /* run a program with arguments main*/
        // execvp(argv[1], arg);

        /**/
        execl("/bin/sh", "sh", "-c", argv[1], NULL);

        perror("execvp");
        _exit(127);
    }

    int status;

    waitpid(pid, &status, 0);

    if (WIFEXITED(status))
    {
        printf("Child exit status = %d\n",
               WEXITSTATUS(status));
    }

    return 0;
}
#elif CASE == 7
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    printf("parent:\n");
    printf("PID  = %d\n", getpid());
    printf("PPID: %d\n", getppid());
    printf("PGID = %d\n", getpgrp());
    printf("--------------------------------\n");

    pid_t pid = fork();

    if (pid == 0)
    {
        printf("child:\n");
        printf("child PID  = %d\n", getpid());
        printf("child PPID: %d\n", getppid());
        printf("child PGID = %d\n", getpgrp());
        printf("--------------------------------\n");

        setpgid(0, 0);
        pid_t pid = fork();

        if (pid == 0)
        {
            while (1)
            {
                printf("grandchild:\n");
                printf("grandchild PID  = %d\n", getpid());
                printf("grandchild PPID: %d\n", getppid());
                printf("grandchild PGID = %d\n", getpgrp());
                printf("--------------------------------\n");
                sleep(1);
            }
        }
        else
        {
            while (1)
            {
                printf("child:\n");
                printf("child PID  = %d\n", getpid());
                printf("child PPID: %d\n", getppid());
                printf("child PGID = %d\n", getpgrp());
                printf("--------------------------------\n");
                sleep(1);
            }
        }
    }
    else
    {
        sleep(7);
        kill(-pid, SIGTERM);
    }

    return 0;
}
#elif CASE == 8
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    printf("Before setsid:\n");
    printf("PID  = %d\n", getpid());
    printf("PPID = %d\n", getppid());
    printf("PGID = %d\n", getpgrp());
    printf("SID  = %d\n", getsid(0));

    printf("--------------------------------\n");

    pid_t pid = fork();

    if (pid == 0)
    {
        printf("Child before setsid:\n");
        printf("PID  = %d\n", getpid());
        printf("PPID = %d\n", getppid());
        printf("PGID = %d\n", getpgrp());
        printf("SID  = %d\n", getsid(0));

        printf("--------------------------------\n");

        setsid();

        printf("Child after setsid:\n");
        printf("PID  = %d\n", getpid());
        printf("PPID = %d\n", getppid());
        printf("PGID = %d\n", getpgrp());
        printf("SID  = %d\n", getsid(0));
    }
    sleep(1);

    return 0;
}
#elif CASE == 9
#include <stdio.h>

int main(void)
{
    while (1)
    {
        /* code */
        printf("Hello\n");
        sleep(1);
    }
}
#elif CASE == 10
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main()
{
    pid_t pid = fork();

    if (pid == 0)
    {
        /* CHILD */
        printf("Child: preparing important data...\n");

        FILE *fp = fopen("important.txt", "w");
        fprintf(fp, "IMPORTANT DATA\n");

        printf("Child: now exec another program...\n");

        /* call exec() */
        execl("/home/hungubuntu/Vim_C_code/test", "test", NULL);

        perror("exec");
        exit(127);
    }
    else
    {
        wait(NULL);
        printf("I am parent.  Child already done!\n");
    }
}
#elif CASE == 11
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <errno.h>

/*
    function print ids of preocess:
        PID
        PPID
        PGID
        SID
        TID
*/
void print_id(const char *name)
{
    printf("\n--- %s ---\n", name);
    printf("PID  = %d\n", getpid());
    printf("PPID = %d\n", getppid());
    printf("PGID = %d\n", getpgid(0));
    printf("SID  = %d\n", getsid(0));
    printf("TID  = %ld\n", (long)getpid());
}

void init_daemon(void)
{
    /* create pipe to communicate with daemon */
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1)
    {
        syslog(LOG_ERR, "pipe failed: %d", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* First fork: Detaches the child process from the parent terminal */
    pid_t pid = fork();

    if (pid < 0)
    {
        /* error */
        syslog(LOG_ERR, "fork failed: %d", strerror(errno));
        _exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        /* Parent wait daemon success and exits */
        close(pipe_fd[1]); /* Close write end of pipe */
        char buffer[1];
        if (read(pipe_fd[0], buffer, 1) > 0)
        {
            printf("\nParent receives success signal. PID = %d\n", getpid());
        }
        close(pipe_fd[0]); /* Close read end of pipe */
        syslog(LOG_INFO, "Parent exits. PID = %d", getpid());
        _exit(EXIT_SUCCESS);
    }

    print_id("CHILD 1 AFTER FORK");
    close(pipe_fd[0]); /* Close read end of pipe in child 1 */

    /* Create a new session and process group (becomes Session Leader) */
    if (setsid() == -1)
    {
        syslog(LOG_ERR, "setsid failed: %d", strerror(errno));
        _exit(EXIT_FAILURE);
    }

    print_id("CHILD 1 AFTER SETSID");

    /*
     * Second fork: daemon is no longer a session leader,
     * so it cannot acquire a controlling terminal.
     */
    pid = fork();

    if (pid < 0)
    {
        /* error */
        syslog(LOG_ERR, "fork failed: %d", strerror(errno));
        _exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        /* child 1 exits */
        close(pipe_fd[1]); /* Close write end of pipe in child 1 */
        syslog(LOG_INFO, "Child 1 exits. PID = %d", getpid());
        _exit(EXIT_SUCCESS);
    }

    sleep(1); /* Ensure child 1 has exited before proceeding */
    print_id("CHILD 2 AFTER FORK");

    /* Redirect stdin/stdout/stderr */
    int fd = open("/dev/null", O_RDWR);

    if (fd < 0)
    {
        syslog(LOG_ERR, "open /dev/null failed: %d", strerror(errno));
        _exit(EXIT_FAILURE);
    }

    if (dup2(fd, STDIN_FILENO) < 0 ||
        dup2(fd, STDOUT_FILENO) < 0 ||
        dup2(fd, STDERR_FILENO) < 0)
    {
        syslog(LOG_ERR, "dup2 failed: %d", strerror(errno));
        _exit(EXIT_FAILURE);
    }
    if (fd > STDERR_FILENO)
        close(fd);

    /* Unmask file permissions */
    umask(0);

    /* Change working directory to root to prevent locking mounted filesystems */
    if (chdir("/") < 0)
    {
        syslog(LOG_ERR, "chdir failed: %d", strerror(errno));
        _exit(EXIT_FAILURE);
    }

    /* Open System Logging */
    openlog("log_daemon", LOG_PID | LOG_NDELAY, LOG_DAEMON);

    /* Signal success to parent */
    char success = '1';
    if (write(pipe_fd[1], &success, 1) != 1)
    {
        syslog(LOG_ERR, "write failed: %d", strerror(errno));
        _exit(EXIT_FAILURE);
    }
    close(pipe_fd[1]); /* Close write end of pipe */
}

int main(void)
{
    print_id("BEFORE FORK");

    init_daemon();

    /* child become daemon and run forever */
    while (1)
    {
        sleep(5);
    }

    return 0;
}
#elif CASE == 12
#include <stdio.h>
/* library for mmap() */
#include <sys/mman.h>
#include <stdlib.h>
/* library for fork() */
#include <unistd.h>

int main(int argc, char *argv[])
{
    int *counter = mmap(NULL,
                        sizeof(int),
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS,
                        -1,
                        0);

    *counter = 0;

    pid_t pid = fork();

    /* error fork */
    if (pid < 0)
    {
        perror("fork");
        exit(-1);
    }

    for (int i = 0; i < 100000; i++)
    {
        (*counter)++;
    }

    if (pid > 0)
    {
        printf("Final counter = %d\n", *counter);
    }

    return 0;
}
#endif
