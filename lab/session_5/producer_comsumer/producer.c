#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>

sem_t *empty; /* producer */
sem_t *full;  /* consumer */
sem_t *mutex; /* lock */

int *ptr;
int fd_shm;

void *routine(void *arg)
{
    while (1)
    {
        sem_wait(empty);
        sem_wait(mutex);

        *(int *)ptr += 10;
        printf("server %d post 10.\n", *(int *)arg);

        sem_post(mutex);
        sem_post(full);
        // sleep(1);
    }
    return NULL;
}

void handle(int sig)
{
    munmap(ptr, 1024);

    close(fd_shm);
    shm_unlink("/shm_data");
    sem_close(empty);
    sem_unlink("/sem_empty");
    sem_close(full);
    sem_unlink("/sem_full");
    sem_close(mutex);
    sem_unlink("/sem_mutex");
    exit(0);
}

int main(void)
{
    signal(SIGINT, handle);

    empty = sem_open("/sem_empty", O_CREAT, 0666, 5);
    full = sem_open("/sem_full", O_CREAT, 0666, 0);
    mutex = sem_open("/sem_mutex", O_CREAT, 0666, 1);

    fd_shm = shm_open("/shm_data", O_CREAT | O_RDWR, 0666);

    if (ftruncate(fd_shm, 1024) == -1)
    {
        perror("ftruncate");
        return -1;
    }

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

    close(fd_shm);
    shm_unlink("/shm_data");
    sem_close(empty);
    sem_unlink("/sem_empty");
    sem_close(full);
    sem_unlink("/sem_full");
    sem_close(mutex);
    sem_unlink("/sem_mutex");

    return 0;
}