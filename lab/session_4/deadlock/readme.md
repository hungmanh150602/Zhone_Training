# In this example, I will create a deadlock scenario involving two threads that acquire two locks in reverse order.

Thread 1 locks A and then locks B, while Thread 2 locks B and then locks A.

Both threads wait for the other to release its lock before they can proceed, resulting in a deadlock.

```c
/* thread 1 run */
void *thread1(void *arg)
{
    pthread_mutex_lock(&mutex_A); /* lock A */

    printf("T1 locked A\n");

    sleep(1);

    printf("T1 waiting for lock B\n");
    pthread_mutex_trylock(&mutex_B); /* lock B*/

    printf("T1 locked B\n");

    pthread_mutex_unlock(&mutex_B);
    pthread_mutex_unlock(&mutex_A);

    return NULL;
}

/* thread 2 run */
void *thread2(void *arg)
{
    pthread_mutex_lock(&mutex_B); /* lock B */

    printf("T2 locked B\n");

    sleep(1);

    printf("T2 waiting for lock A\n");
    pthread_mutex_lock(&mutex_A); /* lock A */

    printf("T2 locked A\n");

    pthread_mutex_unlock(&mutex_A);
    pthread_mutex_unlock(&mutex_B);

    return NULL;
}
```

```text
T1 locked A
T2 locked B
T1 waiting for lock B
T2 waiting for lock A

```

To fix it, I simply need to rearrange the lock order correctly:

For example, locking A and then locking B, applied to both threads:

```text
T1 locked A
T1 waiting for lock B
T1 locked B
T2 locked B
T2 waiting for lock A
T2 locked A
```

The issue has been resolved.