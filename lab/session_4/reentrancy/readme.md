# Thís example I will test the reentrancy (thread-safety) of a self-written function and fix it if it is not reentrant

I wrote a program with four threads that use the same thread-routine function; they all call `get_message` to write a specific string to buffer. However, I used a `static` variable within that function to store the string. This causes other threads to access the same variable and overwrite the data. This makes the function ***not reentrant***.

`get_mesage` function:

```c
char *get_message(int thread_id)
{
    static char buffer[128];

    snprintf(buffer, sizeof(buffer),
             "Thread %d: Hello from get_message()", thread_id);

    return buffer;
}
```

The result when I run the program below:

```text
Thread 1 -> Thread 5: Hello from get_message()
Thread 2 -> Thread 1: Hello from get_message()
Thread 3 -> Thread 2: Hello from get_message()
```

Thread data has been mixed up between threads.

To fix it, I don't use `static` value in this function:

```c
void get_message(int thread_id, char *buffer, size_t size)
{
    snprintf(buffer, size,
             "Thread %d: Hello from get_message()", thread_id);
}
```

And then modify the thread routine function:

```c
void *routine(void *arg)
{
    int thread_id = *(int *)arg;

    for (int i = 0; i < 10; i++)
    {
        char buffer[128];

        get_message(thread_id, buffer, sizeof(buffer));

        usleep(1000);

        printf("Thread %d -> %s\n", thread_id, buffer);
    }

    return NULL;
}
```

The result is:

```text
Thread 1 -> Thread 1: Hello from get_message()
Thread 4 -> Thread 4: Hello from get_message()
Thread 2 -> Thread 2: Hello from get_message()
```

Perfect!!!