#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>

int main(void)
{
    int pipefd[2];
    int available = 0;

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    const char msg[] = "Hello IOCTL";

    if (write(pipefd[1], msg, sizeof(msg) - 1) == -1) {
        perror("write");
        close(pipefd[0]);
        close(pipefd[1]);
        return 1;
    }

    /* FIONREAD / _IOR('G', 1, int) */

    if (ioctl(pipefd[0], FIONREAD, &available) == -1) {
        perror("ioctl");
        close(pipefd[0]);
        close(pipefd[1]);
        return 1;
    }

    printf("Bytes available: %d\n", available);

    close(pipefd[0]);
    close(pipefd[1]);

    return 0;
}