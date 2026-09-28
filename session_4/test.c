/*
CASE 0 : share memory
CASE 1 : test joint return
CASE 2 : detact
CASE 3 : thread id and join each other
CASE 4 : test id
CASE 5 : test cancel thread
CASE 6 : race condition
CASE 7 : thread cleanup
CASE 8 : mutex
CASE 9 : trylock
CASE 10 : Read-Write Lock
CASE 11 : Thread-Specific Data
CASE 12 : Thread and signal
CASE 13 : Multiple multiple thread
CASE 14 : Deadlock
CASE 15 : Thread and fork
CASE 16 : Thread and I/O
CASE 17 : Condition Variables
CASE 18 : Barriers
CASE 19 : Semaphores
CASE 20 : Deadlock
CASE 21 : LiveLock
*/

#define CASE 20

#if CASE == 0
#include <stdio.h>
#include <pthread.h>

int *p1 = NULL;

void *worker1(void *arg)
{
    p1 = malloc(sizeof(int));
    *p1 = 30;

    sleep(5);

    printf("Thread 1 stopped!");

    return NULL;
}

void *worker2(void *arg)
{

    sleep(1);
    printf("%d\n", *p1);
    free(p1);
    printf("Thread 2 stopped!");

    return NULL;
}

int main(void)
{
    pthread_t id1, id2;

    pthread_create(&id1, NULL, worker1, NULL);
    pthread_create(&id2, NULL, worker2, NULL);

    pthread_join(id1, NULL);
    pthread_join(id2, NULL);
    return 0;
}
#elif CASE == 1
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/wait.h>

struct foo
{
    /* data */
    int a, b, c, d;
};

void *fn_thread1(void *arg)
{
    printf("Thread 1: start\n");

    int *rel = malloc(sizeof(int));

    *rel = 123;

    sleep(2);

    printf("Thread 1: end\n");

    return rel;
}

void *fn_thread2(void *arg)
{
    struct foo foo1 = {1, 2, 3, 4};
    // struct foo *foo1 = arg;

    printf("Struct in function thread 2:\n");
    printf(" structure at %p\n", &foo1);
    printf(" foo.a = %d\n", foo1.a);
    printf(" foo.b = %d\n", foo1.b);
    printf(" foo.c = %d\n", foo1.c);
    printf(" foo.d = %d\n", foo1.d);

    pthread_exit((void *)&foo1);
}

int main(void)
{
    pthread_t tid1, tid2;

    /* First thread */
    if (pthread_create(&tid1, NULL, fn_thread1, NULL) != 0)
    {
        printf("Create the first thread fail!\n");
        return 1;
    }

    printf("Main: waiting thread 1 ...\n");

    void *ret = NULL;

    /* join thread 1 */
    if (pthread_join(tid1, &ret) != 0)
    {
        printf("Error join thread 1!\n");
        return 3;
    }

    /* join thread 1 success */
    printf("Thread 1 finished with return value: ");
    printf("%d\n\n", *(int *)ret);

    free(ret);

    /* end first thread, start second thread */
    /* second thread */
    // struct foo foo1 = {1, 2, 3, 4};

    struct foo *fo = NULL;

    printf("Main prepare to create second thread\n");

    if (pthread_create(&tid2, NULL, fn_thread2, NULL) != 0)
    {
        printf("Create the second thread fail!\n");
        return 2;
    }

    /* join thread 2 */
    if (pthread_join(tid2, (void **)&fo) != 0)
    {
        printf("Error join thread 2!\n");
        return 4;
    }

    /* join thread 2 sucess */
    printf("Thread 2 finished with return value: ");
    printf("%d\n", *(int *)fo);

    printf("Struct in main:\n");
    printf("    structure at %p\n", fo);
    printf("    foo.a = %d\n", fo->a);
    printf("    foo.b = %d\n", fo->b);
    printf("    foo.c = %d\n", fo->c);
    printf("    foo.d = %d\n", fo->d);

    return 0;
}
#elif CASE == 2
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

void *delay_1(void *arg)
{
    printf("Thread is delaying 5s ...\n");
    sleep(5);
    printf("Thread stopped!\n");
    return NULL;
}

int main(void)
{
    int ret;

    pthread_t tid1;

    pthread_create(&tid1, NULL, delay_1, NULL);

    pthread_detach(tid1);

    printf("Main still running ...\n");

    ret = pthread_join(tid1, NULL);

    sleep(1);
    printf("%d (%s)\n", ret, strerror(ret));
    printf("Main stopped!\n");

    return 0;
}
#elif CASE == 3
#include <stdio.h>
#include <pthread.h>
#include <string.h>

pthread_t tid[3];

/*
This function will observe a thread join itself
If a thread join itself, `pthread_join` will return error number
*/
void *deadlock(void *arg)
{
    pthread_t my_id = pthread_self();

    /* thread 1 will join itself */
    if (pthread_equal(my_id, tid[0]))
    {
        printf("I am thread 1 and I am waitting for myself to exit!\n");

        int ret = pthread_join(my_id, NULL);

        printf("Thread 1 return value: %d (%s)\n", ret, strerror(ret));
    }
    return NULL;
}

/*
If two threads join each other, it will result in a deadlock.
*/

/*
About this function:
    THREAD 1 will join thread 2
*/
void *deadlock1(void *arg)
{
    printf("I am thread 1 and I am waitting for thread 2 to exit!\n");

    int ret = pthread_join(tid[2], NULL);

    printf("Thread 1 join  return : %d (%s)\n", ret, strerror(ret));

    return NULL;
}

/*
About this function:
    THREAD 2 will join thread 1
*/
void *deadlock2(void *arg)
{
    printf("I am thread 2 and I am waitting for thread 1 to exit!\n");

    int ret = pthread_join(tid[1], NULL);

    printf("Thread 1 join  return : %d (%s)\n", ret, strerror(ret));

    return NULL;
}

int main(void)
{
    /* thread join itself */
    // pthread_create(&tid[0], NULL, deadlock, NULL);
    /*
    two threads join each other
    */
    pthread_create(&tid[1], NULL, deadlock1, NULL);
    pthread_create(&tid[2], NULL, deadlock2, NULL);

    // pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    pthread_join(tid[2], NULL);

    return 0;
}
#elif CASE == 4
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

pthread_t ntid;

void print_ids(const char *s)
{
    pid_t pid;
    pthread_t tid;

    pid = getpid();
    tid = pthread_self();

    printf("%s pid: %lu tid: %lu (0x%lx)\n", s, (unsigned long)pid,
           (unsigned long)tid, (unsigned long)tid);
}

void *routine(void *arg)
{
    print_ids("new thread: ");

    // pthread_exit((void *)0);
    return ((void *)0);
}

int main(void)
{
    print_ids("main thread:");
    int err;

    err = pthread_create(&ntid, NULL, routine, NULL);

    if (err != 0)
        perror("can't create thread");

    sleep(1);

    exit(0);
}
#elif CASE == 5
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

pthread_t thread_id[2];
/*
This function will be implemented by thread 1 and thread 2.
*/
void *rountine(void *arg)
{
    /*
    Thread 1 implement
    */
    if (pthread_equal(pthread_self(), thread_id[0]))
    {
        printf("Thread 1 will sleep in 5 seconds.\n");
        sleep(5);
    }
    /*
    Thread 2 implement
    */
    else if (pthread_equal(pthread_self(), thread_id[1]))
    {
        printf("Thread 2 will cancel thread 1.\n");
        pthread_cancel(thread_id[0]);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    /* create two threads */
    for (int i = 0; i < 2; i++)
    {
        if (pthread_create(&thread_id[i], NULL, rountine, NULL) != 0)
        {
            printf("Create thread %d fail!\n", i + 1);
            return (i + 1);
        }
    }

    /* join thread and get the return value of thread_join */
    for (int i = 0; i < 2; i++)
    {
        int ret = pthread_join(thread_id[i], NULL);
        if (ret != 0)
        {
            printf("Create thread %d fail!\n", i + 1);
            return (i + 1);
        }
        else
        {
            printf("Thread %d exit with return value: %d (%s)\n",
                   i + 1, ret, strerror(ret));
        }
    }

    return 0;
}
#elif CASE == 6
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
    pthread_mutex_init(&mutex, NULL);
    /* I will create 1000 thread */
    int numthread = 1000;
    pthread_t thread_id[numthread];

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

    pthread_mutex_destroy(&mutex);

    return 0;
}
#elif CASE == 7
#include <stdio.h>
#include <pthread.h>

void clean_up(void *arg)
{
    printf("Clean up: %s\n", (char *)arg);
}

void *routine1(void *arg)
{
    printf("Thread 1 strarted.\n");
    pthread_cleanup_push(clean_up, "thread 1 first handler");
    pthread_cleanup_push(clean_up, "thread 1 second handler");
    printf("thread 1 push complete\n");

    if (arg)
    {
        return ((void *)1);
    }

    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);

    return ((void *)1);
}

void *routine2(void *arg)
{
    printf("Thread 2 strarted.\n");
    pthread_cleanup_push(clean_up, "thread 2 first handler");
    pthread_cleanup_push(clean_up, "thread 2 second handler");
    printf("thread 2 push complete\n");

    if (arg)
    {
        pthread_exit((void *)2);
    }

    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);

    pthread_exit((void *)2);
}

int main(int argc, char *argv)
{
    pthread_t thid1, thid2;
    void *ret;

    if (pthread_create(&thid1, NULL, routine1, (void *)1) != 0)
    {
        printf("Create thread 1 fail.\n");
        return 1;
    }

    if (pthread_create(&thid2, NULL, routine2, (void *)1) != 0)
    {
        printf("Create thread 2 fail.\n");
        return 1;
    }

    if (pthread_join(thid1, &ret) != 0)
    {
        printf("Error join thread 1.\n");
        return 111;
    }

    printf("thread 1 exit with value: %d.\n", (int *)(ret));

    if (pthread_join(thid2, &ret) != 0)
    {
        printf("Error join thread 2.\n");
        return 222;
    }

    printf("thread 2 exit with value: %d.\n", (int *)(ret));

    return 0;
}
#elif CASE == 8
#include <stdio.h>
#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/* Thread 1 use mutex to lock critical section */
void *rountine(void *arg)
{
    int *p = arg;
    /* add 1 to x and loop it 1000 times*/
    for (int i = 0; i < 1000; i++)
    {
        pthread_mutex_lock(&mutex);
        *p += 1;
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

/*
Thread don't use mutex and is able to access to critical section
*/
void *rountine1(void *arg)
{
    int *p = arg;
    /* add 1 to x and loop it 1000 times*/
    for (int i = 0; i < 1000; i++)
    {
        *p += 1;
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    int x = 0;
    /* I will create 1000 thread */
    int numthread = 1000;
    pthread_t thread_id[numthread];
    pthread_t id;

    /* create thread */
    for (int i = 0; i < numthread; i++)
    {
        if (pthread_create(&thread_id[i], NULL, rountine, &x) != 0)
        {
            printf("Create thread %d fail.\n", i + 1);
            return (i + 1);
        }
    }

    /* create thread that doesn't use mutex lock */
    pthread_create(&id, NULL, rountine1, &x);

    /* join thread */
    for (int i = 0; i < numthread; i++)
    {
        if (pthread_join(thread_id[i], NULL) != 0)
        {
            printf("join thread %d fail.\n", i + 1);
            return (i + 101);
        }
    }

    /* join thread */
    pthread_join(id, NULL);

    /* ptint out the value of x after 1000 thread run */
    printf("x = %d\n", x);

    return 0;
}
#elif CASE == 9
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
funtion lock
*/
void *thread_function1(void *arg)
{
    printf("[Thread 1] Locked in 2 seconds.\n");
    pthread_mutex_lock(&my_mutex);

    sleep(2);

    pthread_mutex_unlock(&my_mutex);
    printf("[Thread 1] Lock released.\n");
    return NULL;
}

/*
function try lock
*/
void *thread_function2(void *arg)
{
    sleep(0.5);

    for (int i = 0; i < 100; i++)
    {
        int result = pthread_mutex_trylock(&my_mutex);

        /* try lock success */
        if (result == 0)
        {
            printf("[Thread 2] Try lock success at %d times.\n", i + 1);
            pthread_mutex_unlock(&my_mutex);
            break;
        }
        /* lock busy */
        else if (result == EBUSY)
        {
            printf("[Thread 2] Lock busy on attempt %d.\n", i + 1);
            sleep(1);
        }
        /* try lock error */
        else
        {
            printf("[Thread 2] An error occurred.\n");
            break;
        }
    }
    return NULL;
}

int main()
{
    pthread_t thread1, thread2;

    pthread_create(&thread1, NULL, thread_function1, NULL);
    pthread_create(&thread2, NULL, thread_function2, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    pthread_mutex_destroy(&my_mutex);
    return 0;
}
#elif CASE == 10
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
pthread_t thid1[2000], thid2;

int x = 0;

void *reader(void *arg)
{
    printf("Thread read x: ");
    pthread_rwlock_rdlock(&rwlock);
    printf("%d\n", x);
    pthread_rwlock_unlock(&rwlock);

    return NULL;
}

void *writer(void *arg)
{
    printf("Thread will add 10 to x.\n");
    pthread_rwlock_wrlock(&rwlock);
    x += 10;
    pthread_rwlock_unlock(&rwlock);
    printf("Thread write unlock.\n");
    return NULL;
}

int main(int argc, char *argv[])
{
    /* create 10000 thread reader  */
    for (int i = 0; i < 200; i++)
    {
        if (i == 100)
        {
            /* create thread writer */
            if (pthread_create(&thid2, NULL, writer, NULL) != 0)
            {
                printf("Create thread writer fail.\n");
                return -99999;
            }
        }
        if (pthread_create(&thid1[i], NULL, reader, NULL) != 0)
        {
            printf("Create thread %d fail.\n", i + 1);
            return -(i + 1);
        }
    }

    /* join thread reader */
    for (int i = 0; i < 200; i++)
    {
        if (pthread_join(thid1[i], NULL) != 0)
        {
            printf("Join thread %d fail.\n", i + 1);
            return -(i + 1);
        }
    }

    /* join thread writer */
    pthread_join(thid2, NULL);

    return 0;
}
#elif CASE == 11
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

pthread_key_t buffer_key;

/*
test struct
*/
struct data
{
    int number;
    char text[20];
    bool state;
};

/*
function free memory if it allocated by malloc
*/
void destructor(void *ptr)
{
    printf("Free buffer %p\n", ptr);
    free(ptr);
}

/*
write data to the buffer
each thread has specific buffer but same key
*/
void process_data(const char *data)
{
    struct data *buffer = pthread_getspecific(buffer_key);

    snprintf(buffer->text, sizeof(buffer->text), "%s", data);

    printf("Thread %lu: buffer = %p, content = %s\n",
           pthread_self(), (void *)buffer, buffer->text);
}

/*
read data from buffer
each thread has specific buffer but same key
*/
void log_data(void)
{
    struct data *buffer = pthread_getspecific(buffer_key);

    printf("Thread %lu: buffer = %p, content = %s\n",
           pthread_self(), (void *)buffer, buffer->text);
}

/*
routine function use specific data
*/
void *routine(void *arg)
{
    struct data buffer;

    /* add buffer's memory to key slot */
    pthread_setspecific(buffer_key, &buffer);

    printf("Thread %lu: set buffer = %p\n", pthread_self(), (void *)&buffer);

    /* if thread 1: write "Hello1" to buffer  */
    if (*(int *)arg == 1)
    {
        process_data("Hello1");
        sleep(1);
        log_data();
    }
    /* if thread 2: write "Hello2" to buffer */
    else
    {
        process_data("Hello2");
        log_data();
    }

    return NULL;
}

int main(void)
{
    pthread_t t1, t2;
    int id[2] = {1, 2};

    /* create the key */
    pthread_key_create(&buffer_key, NULL);

    /* create thread */
    pthread_create(&t1, NULL, routine, &id[0]);
    pthread_create(&t2, NULL, routine, &id[1]);

    /* join thread */
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    /* delete key */
    pthread_key_delete(buffer_key);

    return 0;
}
#elif CASE == 12
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

int numthread = 5;
int id[] = {1, 2, 3, 4, 5};
pthread_t thread_id[5];
sigset_t set;

/*
function to handle signal
*/
void signal_handler(int sig)
{
    printf("Thread %lu received signal %d\n", pthread_self(), sig);
}

/*
routine function that thread will run
*/
void *routine(void *arg)
{
    int id = *(int *)arg;

    pthread_t my_id = pthread_self();

    /* thread 3 will accept signal */
    if (pthread_equal(my_id, thread_id[2]))
    {
        pthread_sigmask(SIG_UNBLOCK, &set, NULL);
    }

    printf("Thread %d is sleeping.\n", id);
    sleep(10);
    printf("Thread %d is stopped.\n", id);
    return NULL;
}

int main(int argc, char *argv[])
{
    /* create signal */
    signal(SIGTERM, signal_handler);

    /* create signal mask */
    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    /* create thread */
    for (int i = 0; i < numthread; i++)
    {
        if (pthread_create(&thread_id[i], NULL, routine, &id[i]) != 0)
        {
            printf("Create thread %d fail.\n", i + 1);
            return (i + 1);
        }
    }

    sleep(1);
    printf("main thread send signal to process.\n");
    sleep(1);

    /* send signal to current process */
    kill(getpid(), SIGTERM);

    /* join thread */
    for (int i = 0; i < numthread; i++)
    {
        if (pthread_join(thread_id[i], NULL) != 0)
        {
            printf("join thread %d fail.\n", i + 1);
            return (i + 101);
        }
    }

    return 0;
}
#elif CASE == 13
#include <stdio.h>
#include <pthread.h>

void *rountine(void *arg)
{
    getchar();

    return NULL;
}

int main(int argc, char *argv[])
{
    int x = 0;
    /* i will create 1000 thread */
    int numthread = 1000000;
    pthread_t thread_id[numthread];

    /* create thread */
    for (int i = 0; i < numthread; i++)
    {
        if (pthread_create(&thread_id[i], NULL, rountine, &x) != 0)
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

    return 0;
}
#elif CASE == 14
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *thread_function1(void *arg)
{
    pthread_mutex_lock(&mutex);
    printf("thread 1 will kep lock forever :))\n");
    sleep(3);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void *thread_function2(void *arg)
{
    sleep(1);
    printf("thrad 2 will lock again :((\n");
    pthread_mutex_lock(&mutex);
    printf("hehe thread 2 success :))\n");
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t th_id1, th_id2;
    /* create thread */
    if (pthread_create(&th_id1, NULL, thread_function1, NULL) != 0)
    {
        printf("create thread 1 fail.\n");
        return -1;
    }
    if (pthread_create(&th_id2, NULL, thread_function2, NULL) != 0)
    {
        printf("create thread 2 fail.\n");
        return -2;
    }
    /*--------------------------------------------*/

    /* join thread */
    if (pthread_join(th_id1, NULL) != 0)
    {
        printf("join thread 1 fail.\n");
        return -3;
    }
    if (pthread_join(th_id2, NULL) != 0)
    {
        printf("join thread 1 fail.\n");
        return -4;
    }
    /*--------------------------------------------*/
    return 0;
}
#elif CASE == 15
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>

pthread_t id[5];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *routine(void *arg)
{
    pthread_t my_id = pthread_self();

    /* thread 1 lock */
    if (pthread_equal(my_id, id[0]))
    {
        pthread_mutex_lock(&mutex);
        printf("thread 1 lock.\n");

        sleep(15);

        pthread_mutex_unlock(&mutex);
        printf("Thread 1: mutex unlocked.\n");
    }

    /* thread 4 fork() and try to lock -> deadlock */
    if (pthread_equal(my_id, id[3]))
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork");
            return NULL;
        }
        else if (pid == 0)
        {
            printf("Child: trying to lock mutex...\n");
            pthread_mutex_lock(&mutex);

            printf("child lock sucess.\n");
            sleep(3);

            pthread_mutex_unlock(&mutex);
            _exit(0);
        }
        else
        {
            printf("Thread 4: created child PID = %d\n", pid);

            waitpid(pid, NULL, 0);

            printf("Parent: child exited.\n");
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    printf("parent id: %d\n", getpid());

    for (int i = 0; i < 5; i++)
    {
        if (pthread_create(&id[i], NULL, routine, NULL) != 0)
        {
            printf("create thread %d fail.\n", i + 1);
            return -(i + 1);
        }
    }

    for (int i = 0; i < 5; i++)
    {
        if (pthread_join(id[i], NULL) != 0)
        {
            printf("create thread %d fail.\n", i + 1);
            return -(i + 1);
        }
    }
    return 0;
}
#elif CASE == 16
#elif CASE == 17
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

        while (fuel < 40)
        {
            printf("no fuel. waiting...\n");
            pthread_cond_wait(&condFuel, &mutexFuel);
        }
        fuel -= 40;
        printf("get fuel. Now left: %d\n", fuel);

        pthread_mutex_unlock(&mutexFuel);
    }
    return NULL;
}

/*
    while (1)
    {
        pthread_mutex_lock(&mutexFuel);

        if (0)
        {
            // data = pop();
            pthread_mutex_unlock(&mutexFuel);
            break;
        }

        pthread_mutex_unlock(&mutexFuel);
    }
*/

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
#elif CASE == 18
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

pthread_barrier_t barrier;

void *routine(void *arg)
{
    int ids = *(int *)arg;

    printf("Thread %lu: Phase 1\n", pthread_self());
    sleep(ids);

    printf("Thread %lu: finished Phase 1\n", pthread_self());

    pthread_barrier_wait(&barrier);

    printf("Thread %lu: Phase 2\n", pthread_self());

    sleep(ids);
    pthread_barrier_wait(&barrier);

    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t id[5];
    int ids[5];

    /* init */
    pthread_barrier_init(&barrier, NULL, 5);

    /* create thread */
    for (int i = 0; i < 5; i++)
    {
        ids[i] = i + 1;
        if (pthread_create(&id[i], NULL, routine, &ids[i]) != 0)
        {
            printf("Create thread %d fail.\n", i + 1);
            return (i + 1);
        }
    }

    /* join thread */
    for (int i = 0; i < 5; i++)
    {
        if (pthread_join(id[i], NULL) != 0)
        {
            printf("join thread %d fail.\n", i + 1);
            return (i + 101);
        }
    }

    /* destroy */
    pthread_barrier_destroy(&barrier);
    return 0;
}
#elif CASE == 19
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_THREADS 6

sem_t sem;

void *thread1(void *arg)
{
    printf("Thread 1 is doing something...\n");
    sleep(5);
    printf("Thread 1 done\n");
    sem_post(&sem);

    return NULL;
}

void *thread2(void *arg)
{
    sem_wait(&sem);
    printf("Thread 2 is doing something...\n");

    return NULL;
}

void *worker(void *arg)
{
    int id = *(int *)arg;

    printf("Thread %d: waiting...\n", id);
    sleep(2);

    /* critical sections */
    sem_wait(&sem);
    printf("Thread %d: ENTER\n", id);
    sleep(2);
    printf("Thread %d: LEAVE\n", id);
    sem_post(&sem);
    /*---------------------------------*/
    sleep(2);

    return NULL;
}

int main(void)
{
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];

    sem_init(&sem, 0, 3);

    for (int i = 0; i < NUM_THREADS; i++)
    {
        ids[i] = i + 1;

        pthread_create(&threads[i], NULL, worker, &ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&sem);

    return 0;
}
#elif CASE == 20
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

pthread_mutex_t mutex_A = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_B = PTHREAD_MUTEX_INITIALIZER;

void *thread1(void *arg)
{
    pthread_mutex_lock(&mutex_A);

    printf("T1 locked A\n");

    sleep(1);

    printf("T1 waiting for lock B\n");
    pthread_mutex_trylock(&mutex_B);

    printf("T1 locked B\n");

    pthread_mutex_unlock(&mutex_B);
    pthread_mutex_unlock(&mutex_A);

    return NULL;
}

void *thread2(void *arg)
{
    pthread_mutex_lock(&mutex_B);

    printf("T2 locked B\n");

    sleep(1);

    printf("T2 waiting for lock A\n");
    pthread_mutex_lock(&mutex_A);

    printf("T2 locked A\n");

    pthread_mutex_unlock(&mutex_A);
    pthread_mutex_unlock(&mutex_B);

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
#elif CASE == 21
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void *thread1(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&mutex1);

        if (pthread_mutex_trylock(&mutex2) != 0)
        {
            printf("T1: wait T2\n");
            pthread_mutex_unlock(&mutex1);
            sleep(1);
            continue;
        }

        printf("T1: WORK\n");

        pthread_mutex_unlock(&mutex2);
        pthread_mutex_unlock(&mutex1);
    }

    return NULL;
}

void *thread2(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&mutex2);

        if (pthread_mutex_trylock(&mutex1) != 0)
        {
            printf("T2: wait T1\n");
            pthread_mutex_unlock(&mutex2);
            sleep(1);
            continue;
        }

        printf("T2: WORK\n");

        pthread_mutex_unlock(&mutex1);
        pthread_mutex_unlock(&mutex2);
    }

    return NULL;
}

int main(void)
{
    pthread_t t1, t2;

    pthread_create(&t1, NULL, thread1, NULL);
    pthread_create(&t2, NULL, thread2, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return 0;
}
#endif