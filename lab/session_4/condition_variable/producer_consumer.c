#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

int fuel = 0;
pthread_mutex_t mutexFuel;
pthread_cond_t condFuel;

/* producer fill the fuel */
void *fuel_filling(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&mutexFuel);
        fuel += 15;
        printf("filled fuel ... %d\n", fuel);

        pthread_cond_signal(&condFuel);
        pthread_mutex_unlock(&mutexFuel);
        sleep(1);
    }
    return NULL;
}

/* consumer, car, will comsume the fuel if it >= 40 */
void *car(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&mutexFuel);

        while (fuel < 40) /* if fuel < 40, sleep */
        {
            printf("no fuel. waiting...\n");
            pthread_cond_wait(&condFuel, &mutexFuel);
        }
        /* fuel > = 40, get data */
        fuel -= 40;
        printf("get fuel. Now left: %d\n", fuel);

        pthread_mutex_unlock(&mutexFuel);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t id1, id2, id3;

    /* init */
    pthread_mutex_init(&mutexFuel, NULL);
    pthread_cond_init(&condFuel, NULL);

    /* create thread */
    if (pthread_create(&id1, NULL, fuel_filling, NULL) != 0)
    {
        printf("create thread 1 fail.\n");
        return -1;
    }
    if (pthread_create(&id2, NULL, car, NULL) != 0)
    {
        printf("create thread 2 fail.\n");
        return -2;
    }
    if (pthread_create(&id3, NULL, fuel_filling, NULL) != 0)
    {
        printf("create thread 1 fail.\n");
        return -1;
    }

    /* join thread */
    if (pthread_join(id1, NULL) != 0)
    {
        printf("join thread 1 fail.\n");
        return -3;
    }
    if (pthread_join(id2, NULL) != 0)
    {
        printf("join thread 1 fail.\n");
        return -4;
    }
    if (pthread_join(id3, NULL) != 0)
    {
        printf("join thread 1 fail.\n");
        return -4;
    }

    /* destroy */
    pthread_mutex_destroy(&mutexFuel);
    pthread_cond_destroy(&condFuel);
    return 0;
}