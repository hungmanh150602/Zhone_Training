/*
CASE 0 : get limit
CASE 1 : set limit
CASE 2 : tets limit stack
CASE 3 : test limit open file
            fd: 0 → stdin
            fd: 1 → stdout
            fd: 2 → stderr
*/

/* some commands in terminal

ulimit -a : see all
ulimit -n : see the limit of open file
ulimit -s : see the limit of stack size
*/

/*
RLIMIT_NOFILE : limit the number of file descriptors a process can open.
RLIMIT_STACK : process stack limit.
RLIMIT_CPU : the limit on the CPU time used by the process.
RLIMIT_NPROC : the number of processes/threads a user can create 
                                under specific Linux conditions.
*/
#define CASE 0

#if CASE == 0
#include <stdio.h>
#include <sys/resource.h>

int main(void)
{
    struct rlimit limit;

    int ret = getrlimit(RLIMIT_NOFILE, &limit);

    if(ret == -1)
    {
        perror("getrlimit");
        return -1;
    }

    printf("soft limit = %lu\n", (unsigned long)limit.rlim_cur);
    printf("hard limit = %lu\n", (unsigned long)limit.rlim_max);

    ret = getrlimit(RLIMIT_STACK, &limit);

    if(ret == -1)
    {
        perror("getrlimit");
        return -1;
    }

    printf("soft limit = %lu\n", (unsigned long)limit.rlim_cur);
    printf("hard limit = %lu\n", (unsigned long)limit.rlim_max);

    return 0;
}
#elif CASE == 1
#include <stdio.h>
#include <sys/resource.h>

int main(void)
{
    struct rlimit limit;

    limit.rlim_cur = 100;
    limit.rlim_max = 10000;

    if (setrlimit(RLIMIT_NOFILE, &limit) == -1)
    {
        perror("setrlimit");
        return 1;
    }

    printf("limit changed\n");

    int ret = getrlimit(RLIMIT_NOFILE, &limit);

    if(ret == -1)
    {
        perror("getrlimit");
        return -1;
    }

    printf("soft limit = %lu\n", (unsigned long)limit.rlim_cur);
    printf("hard limit = %lu\n", (unsigned long)limit.rlim_max);

    return 0;
}
#elif CASE == 2
#include <stdio.h>

void recursive(int n)
{
    char buffer[1024];

    printf("depth = %d, buffer = %p\n",
           n, (void *)buffer);

    recursive(n + 1);
}

int main(void)
{
    recursive(0);

    return 0;
}
#elif CASE == 3
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/resource.h>

int main(void)
{
    struct rlimit limit;

    limit.rlim_cur = 10;
    limit.rlim_max = 10;

    if (setrlimit(RLIMIT_NOFILE, &limit) == -1)
    {
        perror("setrlimit");
        return 1;
    }

    for (int i = 0; i < 20; i++)
    {
        printf("%d ", i);
        int fd = open("/dev/null", O_RDONLY);

        if (fd == -1)
        {
            perror("open");
            break;
        }

        printf("opened fd = %d\n", fd);
    }

    return 0;
}
#endif