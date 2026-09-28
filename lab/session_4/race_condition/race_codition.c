#include <stdio.h>
#include <pthread.h>

int x = 0;
pthread_mutex_t mutex;

void *rountine(void *arg)
{
    /* add 1 to x and loop it 1000 times*/
    for (int i = 0; i < 1000; i++)
    {
        pthread_mutex_lock(&mutex);
        x += 1;
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    /* init mutex */
    // pthread_mutex_init(&mutex, NULL);

    int numthread = 1000;           /* I will create 1000 thread */
    pthread_t thread_id[numthread]; /* thread id */

    /* create thread and run the routine function with the argument is x */
    for (int i = 0; i < numthread; i++)
    {
        if (pthread_create(&thread_id[i], NULL, rountine, NULL) != 0)
        {
            printf("Create thread %d fail.\n", i + 1);
            return (i + 1);
        }
    }

    /* join thread */
    for (int i = 0; i < numthread; i++)
    {
        if (pthread_join(thread_id[i], NULL) != 0)
        {
            printf("join thread %d fail.\n", i + 1);
            return (i + 101);
        }
    }

    /* ptint out the value of x after 1000 thread run */
    printf("x = %d\n", x);

    // pthread_mutex_destroy(&mutex);

    return 0;
}