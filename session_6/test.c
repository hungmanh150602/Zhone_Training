/*
CASE 0: multi-thread
CASE 1: multi-process
CASE 2: Valgrind
*/

#define CASE 0

#if CASE == 0
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define N 10

int state[N] = {0};

void *worker(void *arg)
{
    int id = *(int *)arg;

    for (int i = 0; i < 5; i++)
    {
        state[id] = i + 1;
        usleep(10000);
    }

    printf("Worker %d done\n", id);
    fflush(stdout);

    return NULL;
}

void *checker(void *arg)
{
    (void)arg;

    int complete = 0;

    while (complete != N)
    {
        complete = 0;

        for (int i = 0; i < N; i++)
        {
            if (state[i] == 5)
                complete++;
        }

        usleep(1000);
    }

    printf("Checker: all workers completed\n");

    return NULL;
}

int main(void)
{
    pthread_t workers[N];
    pthread_t checker_thread;
    int ids[N];

    pthread_create(&checker_thread, NULL, checker, NULL);

    for (int i = 0; i < N; i++)
    {
        ids[i] = i;
        pthread_create(&workers[i], NULL, worker, &ids[i]);
    }

    for (int i = 0; i < N; i++)
        pthread_join(workers[i], NULL);

    pthread_join(checker_thread, NULL);

    printf("Program finished\n");

    return 0;
}
#elif CASE == 1
#elif CASE == 2
#include <stdlib.h>
#include <stdio.h>

int main(void)
{
    int x;

    printf("x = %d\n", x);

    return 0;
}
#endif