#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define NUM_THREADS 5

/*
 * A static buffer exists throughout the lifetime of the process.
 * and all threads share this buffer.
 */
char *get_message(int thread_id)
{
    static char buffer[128];

    snprintf(buffer, sizeof(buffer),
             "Thread %d: Hello from get_message()", thread_id);

    /*
     * Allow other threads to run
     * and overwrite the buffer.
     */
    usleep(1000);

    return buffer;
}

void *routine(void *arg)
{
    int thread_id = *(int *)arg;

    for (int i = 0; i < 10; i++)
    {
        char *msg = get_message(thread_id);

        /*
         * The thread might be switched to another thread by the scheduler
         * before printf().
         */
        usleep(1000);

        printf("Thread %d -> %s\n", thread_id, msg);
    }

    return NULL;
}

int main(void)
{
    pthread_t threads[NUM_THREADS];
    int thread_id[NUM_THREADS];

    /* create thread */
    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_id[i] = i + 1;

        if (pthread_create(&threads[i], NULL, routine, &thread_id[i]) != 0)
        {
            perror("pthread_create");
            return 1;
        }
    }

    /* join */
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return 0;
}