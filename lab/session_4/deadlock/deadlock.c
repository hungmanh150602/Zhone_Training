#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

pthread_mutex_t mutex_A = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_B = PTHREAD_MUTEX_INITIALIZER;

/* thread 1 run */
void *thread1(void *arg)
{
    pthread_mutex_lock(&mutex_A); /* lock A */

    printf("T1 locked A\n");

    sleep(1);

    printf("T1 waiting for lock B\n");
    pthread_mutex_lock(&mutex_B); /* lock B*/

    printf("T1 locked B\n");

    pthread_mutex_unlock(&mutex_B);
    pthread_mutex_unlock(&mutex_A);

    return NULL;
}

/* thread 2 run */
void *thread2(void *arg)
{
    pthread_mutex_lock(&mutex_A); /* lock B */

    printf("T2 locked B\n");

    sleep(1);

    printf("T2 waiting for lock A\n");
    pthread_mutex_lock(&mutex_B); /* lock A */

    printf("T2 locked A\n");

    pthread_mutex_unlock(&mutex_B);
    pthread_mutex_unlock(&mutex_A);

    return NULL;
}

int main(void)
{
    pthread_t thread_1, thread_2;

    pthread_create(&thread_1, NULL, thread1, NULL);
    pthread_create(&thread_2, NULL, thread2, NULL);

    pthread_join(thread_1, NULL);
    pthread_join(thread_2, NULL);

    return 0;
}