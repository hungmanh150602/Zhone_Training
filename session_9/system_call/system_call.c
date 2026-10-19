#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <string.h>

// int main(void)
// {
//     long pid = syscall(SYS_getpid);

//     if (pid == -1)
//     {
//         fprintf(stderr, "syscall failed: %s\n", strerror(errno));
//         return 1;
//     }

//     printf("PID from syscall(): %ld\n", pid);
//     printf("PID from getpid():  %d\n", getpid());

//     return 0;
// }

int main(void)
{
    const char message[] = "Hello from syscall!\n";
    size_t len = sizeof(message) - 1;
    size_t written = 0;

    while (written < len)
    {
        long ret = syscall(SYS_write, STDOUT_FILENO,
                           message + written, len - written);

        if (ret == -1)
        {
            if (errno == EINTR)
                continue;

            perror("syscall write");
            return 1;
        }

        if (ret == 0)
        {
            fprintf(stderr, "write returned 0\n");
            return 1;
        }

        written += (size_t)ret;
    }

    return 0;
}