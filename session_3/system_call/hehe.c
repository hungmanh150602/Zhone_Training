#include <stdio.h>
#include <sys/wait.h>

int main()
{
    printf("hehe\n");
    kill(-11294, SIGTERM);
    return 0;
}