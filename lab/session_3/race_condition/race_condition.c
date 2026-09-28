#include <stdio.h>
/* library for mmap() */
#include <sys/mman.h>
#include <stdlib.h>
/* library for fork() */
#include <unistd.h>

int main(int argc, char *argv[])
{
    /* create an pointer points to the shared memory region */
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

    /*
        Both the parent and child processes access the same memory region.    
    */
    for (int i = 0; i < 100000; i++)
    {
        (*counter)++;
    }

    /* parent prints out the value of counter after done */
    if (pid > 0)
    {
        printf("Final counter = %d\n", *counter);
    }

    return 0;
}