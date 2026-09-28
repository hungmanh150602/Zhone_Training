#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>

sem_t *empty;
sem_t *full;
sem_t *mutex;

int *ptr;
int fd_shm;

void *routine(void *arg)
{
    while (1)
    {
        sem_wait(full);
        sem_wait(mutex);

        *ptr -= 10;
        printf("client %d consume 10. Now left: %d\n", *(int *)arg, *ptr);

        sem_post(mutex);
        sem_post(empty);
        sleep(1);
    }
    return NULL;
}

void handle(int sig)
{
    munmap(ptr, 1024);
    sem_close(empty);
    sem_close(full);
    sem_close(mutex);
    close(fd_shm);
    exit(0);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, handle);

    empty = sem_open("/sem_empty", 0);
    full = sem_open("/sem_full", 0);
    mutex = sem_open("/sem_mutex", 0);

    fd_shm = shm_open("/shm_data", O_RDWR, 0666);

    ptr = mmap(NULL, 1024, PROT_READ | PROT_WRITE,
               MAP_SHARED, fd_shm, 0);

    pthread_t thr[7];
    int id[7] = {1, 2, 3, 4, 5, 6, 7};

    for (int i = 0; i < 7; i++)
    {
        if (pthread_create(&thr[i], NULL, routine, &id[i]) == -1)
        {
            printf("create thread %d fail.\n", i + 1);
            return -1;
        }
    }

    for (int i = 0; i < 7; i++)
    {
        if (pthread_join(thr[i], NULL) == -1)
        {
            printf("join thread %d fail.\n", i + 1);
            return -1;
        }
    }

    munmap(ptr, 1024);
    sem_close(empty);
    sem_close(full);
    sem_close(mutex);
    close(fd_shm);

    return 0;
}