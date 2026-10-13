#include <stdio.h>
#include <unistd.h>
#include <errno.h>

int main(void)
{
    char buf[128];

    printf("Nhap mot dong: ");
    fflush(stdout);

    ssize_t n = read(STDIN_FILENO, buf, sizeof(buf));

    if (n < 0) {
        perror("read");
        return 1;
    }

    if (n == 0) {
        printf("EOF\n");
        return 0;
    }

    printf("Da doc %zd byte\n", n);

    return 0;
}