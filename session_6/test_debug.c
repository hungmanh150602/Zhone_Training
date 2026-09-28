#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 4
#define NUM_ITERATIONS 100000

typedef struct {
    int *data;
    int size;
} SharedBuffer;

SharedBuffer buffer;

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;


/* ---------------------------------------------------------
 * Initialize shared buffer
 * --------------------------------------------------------- */
void buffer_init(int size)
{
    buffer.data = malloc(sizeof(int) * size);
    buffer.size = size;

    if (buffer.data == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < size; i++) {
        buffer.data[i] = i;
    }
}


/* ---------------------------------------------------------
 * Destroy shared buffer
 * --------------------------------------------------------- */
void buffer_destroy(void)
{
    free(buffer.data);

    /*
     * Deliberately separated from free().
     * Another thread may access buffer.data
     * before it is changed to NULL.
     */
    buffer.data = NULL;
    buffer.size = 0;
}


/* ---------------------------------------------------------
 * Thread that reads the shared buffer
 * --------------------------------------------------------- */
void *reader_thread(void *arg)
{
    long id = (long)arg;

    for (int i = 0; i < NUM_ITERATIONS; i++) {

        /*
         * Deliberately slow down execution.
         * This makes the race easier to reproduce.
         */
        if (i % 1000 == 0) {
            usleep(1);
        }

        /*
         * Shared data is accessed without a mutex.
         */
        int index = i % buffer.size;

        int value = buffer.data[index];

        if (i % 20000 == 0) {
            pthread_mutex_lock(&print_mutex);

            printf(
                "Thread %ld: index=%d value=%d\n",
                id,
                index,
                value
            );

            pthread_mutex_unlock(&print_mutex);
        }
    }

    return NULL;
}


/* ---------------------------------------------------------
 * Thread that destroys the shared buffer
 * --------------------------------------------------------- */
void *destroyer_thread(void *arg)
{
    (void)arg;

    /*
     * Wait so reader threads have time to start.
     */
    usleep(5000);

    printf("DESTROYER: freeing shared buffer!\n");

    buffer_destroy();

    printf("DESTROYER: buffer destroyed.\n");

    return NULL;
}


/* ---------------------------------------------------------
 * Create reader threads
 * --------------------------------------------------------- */
void create_readers(pthread_t threads[])
{
    for (long i = 0; i < NUM_THREADS; i++) {

        int ret = pthread_create(
            &threads[i],
            NULL,
            reader_thread,
            (void *)i
        );

        if (ret != 0) {
            fprintf(stderr, "pthread_create failed\n");
            exit(EXIT_FAILURE);
        }
    }
}


/* ---------------------------------------------------------
 * Wait for reader threads
 * --------------------------------------------------------- */
void join_readers(pthread_t threads[])
{
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
}


/* ---------------------------------------------------------
 * Main
 * --------------------------------------------------------- */
int main(void)
{
    pthread_t readers[NUM_THREADS];
    pthread_t destroyer;

    printf("Initializing shared buffer...\n");

    buffer_init(100);

    printf("Creating reader threads...\n");

    create_readers(readers);

    /*
     * Start a thread that frees the buffer
     * while reader threads are still using it.
     */
    pthread_create(
        &destroyer,
        NULL,
        destroyer_thread,
        NULL
    );

    pthread_join(destroyer, NULL);

    printf("Destroyer thread finished.\n");

    join_readers(readers);

    printf("All reader threads finished.\n");

    return 0;
}
