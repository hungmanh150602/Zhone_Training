#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <time.h>
#include <semaphore.h>

#include "common.h"

int main(int argc, char *argv[])
{
    int id;
    int shm_fd;
    struct shared_data *data;
    sem_t *sem;

    if (argc != 2)
    {
        printf("Usage: %s <worker_id>\n", argv[0]);
        return -1;
    }

    id = atoi(argv[1]);

    if (id < 0 || id >= MAX_WORKERS)
    {
        printf("Invalid worker ID\n");
        return -1;
    }

    /* open semaphore */
    sem = sem_open(SEM_NAME, 0);

    /* Open shared memory created by monitor */
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);

    if (shm_fd == -1)
    {
        perror("shm_open");
        return -1;
    }

    /* Map shared memory */
    data = mmap(NULL, sizeof(struct shared_data), PROT_READ | PROT_WRITE,
                MAP_SHARED, shm_fd, 0);

    if (data == MAP_FAILED)
    {
        perror("mmap");
        close(shm_fd);
        return -1;
    }

    close(shm_fd);

    printf("Worker %d started, PID=%d\n", id, getpid());

    /* Register worker */
    sem_wait(sem);

    data->workers[id].pid = getpid();
    data->workers[id].id = id;
    data->workers[id].last_heartbeat = time(NULL);
    data->workers[id].state = WORKER_RUNNING;

    sem_post(sem);

    /* Heartbeat loop */
    while (1)
    {
        sem_wait(sem);

        data->workers[id].last_heartbeat = time(NULL);
        data->workers[id].state = WORKER_RUNNING;

        sem_post(sem);

        printf("Worker %02d heartbeat\n", id);

        sleep(1);
    }

    munmap(data, sizeof(struct shared_data));
    sem_close(sem);
    return 0;
}
