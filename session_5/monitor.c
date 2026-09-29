#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#include "common.h"

static struct shared_data *data;
static int shm_fd;

static void cleanup(void)
{
    if (data != MAP_FAILED)
    {
        munmap(data, sizeof(struct shared_data));
    }

    if (shm_fd != -1)
    {
        close(shm_fd);
    }

    shm_unlink(SHM_NAME);
}

static const char *state_to_string(enum worker_state state)
{
    switch (state)
    {
    case WORKER_RUNNING:
        return "RUNNING";

    case WORKER_HUNG:
        return "HUNG";

    case WORKER_DEAD:
        return "DEAD";

    default:
        return "UNKNOWN";
    }
}

int main(void)
{
    time_t now;
    sem_t *sem;

    /* open semaphore */
    sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);

    /* Remove old shared memory */
    shm_unlink(SHM_NAME);

    /* Create shared memory */
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);

    if (shm_fd == -1)
    {
        perror("shm_open");
        return 1;
    }

    /* Set shared memory size */
    if (ftruncate(shm_fd, sizeof(struct shared_data)) == -1)
    {
        perror("ftruncate");
        cleanup();
        return 1;
    }

    /* Map shared memory */
    data = mmap(NULL, sizeof(struct shared_data), PROT_READ | PROT_WRITE,
                MAP_SHARED, shm_fd, 0);

    if (data == MAP_FAILED)
    {
        perror("mmap");
        cleanup();
        return 1;
    }

    /* Initialize shared memory */
    for (int i = 0; i < MAX_WORKERS; i++)
    {
        data->workers[i].pid = 0;
        data->workers[i].id = i;
        data->workers[i].last_heartbeat = 0;
        data->workers[i].state = WORKER_UNKNOWN;
    }

    printf("====================================\n");
    printf("       PROCESS MONITOR STARTED\n");
    printf("====================================\n");

    while (1)
    {
        sleep(1);

        now = time(NULL);

        sem_wait(sem);

        /*
         * Clear the terminal screen and move the cursor to the top-left corner.
         * \033   : ESC (Escape).
         * \033[H : Move the cursor to the Home position.
         * \033[J : Erase in Display.
         */
        printf("\033[H\033[J");

        printf("============================================\n");
        printf("             PROCESS MONITOR\n");
        printf("============================================\n");

        printf("%-8s %-8s %-12s %-12s\n", "ID", "PID", "TIME", "STATE");

        printf("--------------------------------------------\n");

        for (int i = 0; i < MAX_WORKERS; i++)
        {
            struct worker_info *w = &data->workers[i];

            /* Worker chưa register */
            if (w->pid == 0)
            {
                printf("%-8d %-8s %-12s %-12s\n", i, "-", "-", "UNKNOWN");
                continue;
            }

            /* Check whether process still exists */
            if (kill(w->pid, 0) == -1)
            {
                if (errno == ESRCH)
                {
                    w->state = WORKER_DEAD;
                }
            }
            else
            {
                /*  Check heartbeat. */
                time_t age = now - w->last_heartbeat;

                if (age >= HUNG_TIMEOUT)
                {
                    w->state = WORKER_HUNG;
                }
                else
                {
                    w->state = WORKER_RUNNING;
                }
            }

            time_t age = now - w->last_heartbeat;

            printf("%-8d %-8d %-12ld %-12s\n", w->id, w->pid, age,
                   state_to_string(w->state));
        }

        printf("--------------------------------------------\n");
        printf("Press Ctrl+C to stop monitor\n");

        sem_post(sem);
    }

    sem_close(sem);
    sem_unlink(SEM_NAME);
    cleanup();
    return 0;
}
