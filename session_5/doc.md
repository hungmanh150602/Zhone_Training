# POSIX Message Queue

**LIBRARY**

```c
#include <mqueue.h>
```

Some APIs in Message Queue:

|APIs|----|Effect|
|:---|:---|:---|
|`mq_open()`||create/open message queue|
|`mq_close()`||close the connection to queue|
|`mq_unlink()`||removes a message queue name and marks it for deletion|
|`mq_send()`||send message to queue|
|`mq_receive()`||read message from queue|
|`mq_getattr()`|||
|`mq_setattr()`|||
|`mq_notify()`||subscribe to receive message notifications from a queue.|

**Life circle of message queue:**

```text
mq_open()
    ↓
mq_send() / mq_receive()
    ↓
mq_getattr() / mq_setattr()
    ↓
mq_close()
    ↓
mq_unlink()
```

## mp_open()

If we want to ***open*** the exsited queue, use:

```c
mqd_t mq_open(const char *name, int oflag);
```

And if we want to ***create*** queue, use:

```c
mqd_t mq_open(const char *name, int oflag, mode_t mode,
                     struct mq_attr *attr);
```

### Some commonly flag

|Flag|----|Mean|
|:---|:---|:---|
|`O_RDONLY`||only read data|
|`O_WRONLY`||only write data|
|`O_RDWR`||can read and write data|
|`O_CREAT`||create queue if it does not exist|
|`O_EXCL`||if queue already exist, `mq_open` will return with error `EEXIST`|
|`O_NONBLOCK`||no blocking when queue is empty or full|

### Permission

`mode` argument is a number that determine the permission when the process open the queue.

Example: `mode = 0666`

Each number is ​​represent for an object's permission when access to a message queue.

|0|6|6|6|
|:---|:---|:---|:---|
|other mode|user|group|others|

```text
rwx rwx rwx = 111 111 111
rw- rw- rw- = 110 110 110
rwx --- --- = 111 000 000

and so on...

rwx = 111 in binary = 7
rw- = 110 in binary = 6
r-x = 101 in binary = 5
r-- = 100 in binary = 4
```

Where of course, `r` stands for read and `w` for write then `x` means execute.  
So `6` is read and write.

### Attribute

This is the queue configuration when creating a new queue.

```c
struct mq_attr attr;
```

```c
struct mq_attr
{
  __syscall_slong_t mq_flags; /* Message queue flags.  */
  __syscall_slong_t mq_maxmsg; /* Maximum number of messages.  */
  __syscall_slong_t mq_msgsize; /* Maximum message size.  */
  __syscall_slong_t mq_curmsgs; /* Number of messages currently queued.  */
};
```

|Element|----|Mean|
|:---|:---|:---|
|`mq_flags`|||
|`mq_maxmsg`||the maximum number of messages in queue|
|`mq_msgsize`||the maximum size of each message|
|`mq_curmsgs`||the number of current messages|

## mq_notify()

Register to have the kernel notify the process when a new message appears in the queue.

Prototype:

```c
int mq_notify(mqd_t mqdes, const struct sigevent *notification);
```

Return:

- 0 : success
- 1 : error

The `sigev_notify` field of the `sigevent` structure (pointed to by `sevp`) determines how notification is performed. This field takes one of the following values:

||||
|:---|:---|:---|
|`SIGEV_SIGNAL`||The kernel sends a signal to the process when a notification occurs.|
|`SIGEV_THREAD`||When the message is delivered, call `sigev_notify_function` as if it were the start function of a new thread.|
|`SIGEV_NONE`||Do not send a notification.|

Example:

```c
/*SIGEV_SIGNAL*/
struct sigevent sev;

sev.sigev_notify = SIGEV_SIGNAL;
sev.sigev_signo = SIGUSR1;

mq_notify(mq, &sev);
/*--------------------------------*/

/*SIGEV_THREAD*/
void notification_function(union sigval value)
{
    printf("New message!\n");
}

struct sigevent sev = {0};

sev.sigev_notify = SIGEV_THREAD;
sev.sigev_notify_function = notification_function;

mq_notify(mq, &sev);
/*--------------------------------*/

/*SIGEV_NONE*/
sev.sigev_notify = SIGEV_NONE;
/*--------------------------------*/
```

# POSIX Shared Memory

```bash
ls -l /dev/shm
```

| API            | Chức năng                         |
| -------------- | --------------------------------- |
| `shm_open()`   | Tạo/mở POSIX shared-memory object |
| `ftruncate()`  | Đặt kích thước object             |
| `mmap()`       | Map object vào virtual address    |
| `munmap()`     | Unmap khỏi process                |
| `close()`      | Đóng file descriptor              |
| `shm_unlink()` | Xóa tên shared-memory object      |

Flow:

```text
shm_open
    ↓
ftruncate
    ↓
mmap
    ↓
      USE SHARED MEMORY
    ↓
munmap
    ↓
close
    ↓
shm_unlink
```

So sánh System V và POSIX

```text
System V                    POSIX

shmget()              ≈     shm_open()
   ↓
shmat()                ≈     mmap()
   ↓
shmdt()                ≈     munmap()
   ↓
shmctl(IPC_RMID)      ≈     shm_unlink()
```

## shm_open()

```text
                 shm_open()
                     │
        ┌────────────┼────────────┐
        ▼            ▼            ▼
      name         oflag         mode
        │            │            │
   "/my_shm"   O_CREAT|O_RDWR    0666
```

```text
User process
     │
     │ shm_open("/my_shm", ...)
     ▼
  syscall / libc
     │
     ▼
   Kernel
     │
     ├── tìm SHM object
     │
     ├── nếu O_CREAT:
     │      tạo nếu chưa tồn tại
     │
     ├── kiểm tra permission
     │
     ├── tạo/open file description
     │
     └── cấp fd
     │
     ▼
User process
     │
     └── fd = 3
```

## mmap()

Prototype:

```c
void *mmap(
    void *addr,
    size_t length,
    int prot,
    int flags,
    int fd,
    off_t offset
);
```

### addr

addr = `NULL` Nghĩa là: Kernel tự chọn virtual address phù hợp.

### flags = MAP_SHARED and flags = MAP_PRIVATE

Đây là 2 flag cực kỳ quan trọng.

Khác biệt cốt lõi:

```text
MAP_SHARED
    ↓
Thay đổi của process có thể được nhìn thấy
bởi các mapping khác của cùng object.

MAP_PRIVATE
    ↓
Thay đổi của process không được chia sẻ
theo kiểu đó → copy-on-write.
```

***MAP_SHARED***

>Modification đối với mapping được chia sẻ với các mapping khác của cùng object.

```text
Process A                    Process B

   ptrA                         ptrB
     │                            │
     │                            │
     └──────────┐    ┌────────────┘
                │    │
                ▼    ▼
          +----------------+
          | shared pages   |
          |                |
          | value = 100    |
          +----------------+
```

***MAP_PRIVATE***

Ban đầu:

```text
Process A                    Process B

   ptrA                         ptrB
     │                            │
     │                            │
     └──────────┐    ┌────────────┘
                │    │
                ▼    ▼
          +----------------+
          | original page  |
          | value = 100    |
          +----------------+
```

Cả hai ban đầu đọc: `100`

Nhưng A ghi:

```c
*ptrA = 200;
```

Kernel không muốn A thay đổi page dùng chung theo semantics của MAP_PRIVATE.

Nó thực hiện Copy-on-Write.

Before write:

```text
A ──────┐
        │
        ▼
   +---------+
   | page 100 |
   +---------+
        ▲
        │
B ──────┘
```

Khi A ghi:

```text
A ─────────> +---------+
             | page 200 |
             +---------+

B ─────────> +---------+
             | page 100 |
             +---------+
```

***MAP_SHARED | MAP_ANONYMOUS***

```text
mmap(MAP_SHARED | MAP_ANONYMOUS)
              │
              ▼
          fork()
           /   \
          /     \
       Parent  Child
```

### phân biệt virtual memory, physical memory, page fault, và đặc biệt là tại sao mmap() có thể map lớn hơn RAM.

Giả sử máy bạn có:

RAM = 8 GB

Bạn làm:

```text
void *p = mmap(
    NULL,
    4ULL * 1024 * 1024 * 1024,
    PROT_READ | PROT_WRITE,
    MAP_PRIVATE | MAP_ANONYMOUS,
    -1,
    0
);
```

Bạn vừa yêu cầu 4 GB mapping.

>Điều đó không có nghĩa kernel lập tức lấy 4 GB RAM.

Ban đầu có thể hình dung:

``` text
Virtual Address Space
+--------------------------------+
|                                |
|       4 GB mapped region       |
|                                |
+--------------------------------+

Physical RAM
+--------------------------------+
|       chưa cần 4 GB           |
+--------------------------------+
```

>mmap() trước hết thiết lập virtual memory mapping.

**Khi nào RAM thực sự được sử dụng?**

Khi CPU thực sự truy cập vào một page.

CPU truy cập:

```text
virtual address
      |
      v
+-------------+
| Page Table  |
+-------------+
      |
      v
Có physical page chưa?
      |
   +--+--+
   |     |
  YES    NO
   |     |
   |     v
   |  PAGE FAULT
   |     |
   |     v
   |  kernel cấp/map page
   |     |
   +-----+
      |
      v
physical RAM
```

**"*Page fault*" không nhất thiết là lỗi**

Tên "page fault" dễ gây hiểu nhầm.

Có 3 loại khái niệm:

- **Minor/valid page fault**

>Kernel xử lý được.  
Ví dụ page chưa được map vào physical RAM nhưng mapping hợp lệ.

- **Major page fault**

>Kernel phải lấy dữ liệu từ backing storage, ví dụ disk/file.

- **Invalid page fault**

>Truy cập vùng memory không hợp lệ: `SIGSEGV`

### SIGSEGV vs SIGBUS

Giả sử:

```c
ftruncate(fd, 4096);
```

Object chỉ có:

>4096 bytes

nhưng bạn map:

```c
mmap(NULL, 8192, ...);
```

Mapping có thể được tạo, nhưng nếu bạn truy cập vùng vượt backing object:

```c
ptr[5000] = 'A';
```

có thể nhận:

>SIGBUS

Điểm này khác với việc đơn giản truy cập một địa chỉ hoàn toàn không thuộc mapping, thường dẫn tới:

>SIGSEGV

```text
SIGSEGV
    ↓
memory access không hợp lệ
    ↓
ví dụ mapping không tồn tại / permission sai
```

Trong khi:

```text
SIGBUS
    ↓
mapping tồn tại nhưng backing object
không thể cung cấp dữ liệu tương ứng
```

# POSIX Semaphore

**POSIX semaphore là gì?**

>POSIX semaphore là một counter dùng để đồng bộ hóa giữa các thread hoặc process.

```text
             semaphore
                 |
                 v
             +-------+
             | value |
             |   2   |
             +-------+
              /     \
             /       \
       sem_wait    sem_post
          |             |
       value--        value++
```

**Hai loại POSIX semaphore**

Có hai loại chính:

***A. Unnamed semaphore***

```c
#include <semaphore.h>

int sem_init(sem_t *sem, int pshared, unsigned int value);

int sem_destroy(sem_t *sem);

int sem_wait(sem_t *sem);

int sem_trywait(sem_t *sem);

int sem_timedwait(sem_t *sem,
                  const struct timespec *abs_timeout);

int sem_post(sem_t *sem);

int sem_getvalue(sem_t *sem, int *sval);
```

Tạo trực tiếp trong memory:

```c
sem_t sem;

sem_init(&sem, 0, 1);
```

Dùng chủ yếu cho:

- thread ↔ thread
- process ↔ process nếu đặt semaphore trong shared memory

***B. Named semaphore***

```c
sem_t *sem_open(const char *name,
                int oflag,
                ...);

int sem_close(sem_t *sem);

int sem_unlink(const char *name);
```

Semaphore có tên trong hệ thống:

```c
sem_open("/my_sem", O_CREAT, 0666, 1);
```

Các process khác có thể mở cùng semaphore:

```c
sem_open("/my_sem", 0);
```

## Unnamed semaphore

### sem_init()

```text
             sem_init()
                |
       +--------+--------+
       |        |        |
      sem     pshared   value
```

>pshared = 0: Semaphore được dùng để đồng bộ các thread trong cùng một process.
>pshared != 0: Semaphore có thể được sử dụng để đồng bộ giữa các process.

Example with `pshared != 0`:

Semaphore phải được đặt trong một vùng memory mà hai process cùng map đến cùng physical pages.

```c
sem_t *sem;

sem = mmap(
    NULL,
    sizeof(sem_t),
    PROT_READ | PROT_WRITE,
    MAP_SHARED | MAP_ANONYMOUS,
    -1,
    0
);

sem_init(sem, 1, 0);

pid_t pid = fork();
```

After `mmap` we have `semaphore` in the shared memory which processes can observe.

```text
             Shared memory
          ┌─────────────────┐
          │                 │
          │     sem_t       │
          │                 │
          └────────┬────────┘
                   │
             same physical
                 pages
             /           \
            /             \
           v               v

        Parent           Child
      sem_wait()       sem_post()
```

**Why we see `MAP_ANONYMOUS` and `-1` in the `mmap` function?**

Thông thường, mmap() có thể map một file vào virtual memory, Nhưng MAP_ANONYMOUS thay đổi chuyện này.
Ta dùng `MAP_ANONYMOUS` nghĩa là:

>Mapping này không backed bởi một file.

Nó là vùng memory do kernel cung cấp cho process. Vì vậy `fd` không còn có ý nghĩa.

>Trong ví dụ parent-child này, `MAP_ANONYMOUS | MAP_SHARED` là cách rất tiện để tạo shared memory không cần file, đặc biệt khi các process có quan hệ `fork()`.
>>Nếu bạn muốn hai process độc lập, được khởi động riêng biệt và không có `fork()` chung, thì *anonymous mapping* không phải cách phù hợp để chúng tự tìm thấy cùng vùng memory. Khi đó ta thường dùng POSIX shared memory ***(shm_open + mmap(MAP_SHARED))*** hoặc System V shared memory để hai process cùng mở/attach một vùng shared memory.

### sem_trywait()

```text
sem_wait()
    |
    +-- value > 0 → decrement → return 0
    |
    +-- value == 0 → BLOCK


sem_trywait()
    |
    +-- value > 0 → decrement → return 0
    |
    +-- value == 0 → KHÔNG BLOCK
                     |
                     +→ return -1
                        errno = EAGAIN
```

### sem_timedwait()

Prototype:

```c
int sem_timedwait(
    sem_t *sem,
    const struct timespec *abs_timeout
);
```

Nó nằm giữa:

```text
sem_wait()        sem_trywait()
     |                  |
     |                  |
block vô hạn       không block


sem_timedwait(): wait trong một khoảng thời gian
```

>Một điểm rất dễ nhầm: `abs_timeout`, chữ `abs = absolute`. Nó không phải `"wait 5 seconds"` mà là `"wait until CLOCK_REALTIME reaches this timestamp"`

Example:

```c
struct timespec ts;

clock_gettime(CLOCK_REALTIME, &ts);

ts.tv_sec += 5;

sem_timedwait(&sem, &ts);
```

Nếu semaphore vẫn chưa available sau thời điểm đó thì: `errno == ETIMEDOUT`

## Named Semaphore

```text
Unnamed                    Named

sem_init()                 sem_open()
sem_destroy()              sem_close()
                           sem_unlink()
```

### sem_open()

Prototype:

```c
sem_t *sem;

sem = sem_open("/my_sem", O_CREAT, 0666, value);
```

`value` chỉ có tác dụng khi tạo mới. Nếu `/my_se`m chưa tồn tại `value = 5`. Nhưng nếu `/my_sem` đã tồn tại thì không reset nó thành 5.

***Lifecycle:***

```text
sem_open()
    ↓
OPEN
    ↓
sem_wait / sem_post
    ↓
sem_close()
    ↓
process no longer uses it

sem_unlink()
    ↓
name removed
```

## Semaphore trong Producer–Consumer

Đây là use case quan trọng nhất cần nắm.

Giả sử buffer có: `capacity = 5`

Ta có:

```c
sem_t empty;
sem_t full;
```

Khởi tạo:

```c
sem_init(&empty, 0, 5); /* empty = số slot trống */
sem_init(&full, 0, 0); /* full  = số item hiện có */
```

Ban đầu:

```text
Buffer
+---+---+---+---+---+
|   |   |   |   |   |
+---+---+---+---+---+
 ^
 empty = 5
 full  = 0
```

Producer:

```c
sem_wait(&empty);
sem_wait(&mutex);

put_item();

sem_post(&mutex);
sem_post(&full);
```

Consumer:

```c
sem_wait(&full);
sem_wait(&mutex);

get_item();

sem_post(&mutex);
sem_post(&empty);
```

- Nếu buffer đầy: `empty = 0`, producer: `sem_wait(&empty);` → block.

- Nếu buffer rỗng: `full = 0`, consumer: `sem_wait(&full);` → block.

Đây chính là logic semaphore mà bạn đã gặp khi học shared memory + semaphore.

# POSIX Signal

**What is a signal?**

In Linux, a signal is an asynchronous notification mechanism between the kernel and a process.

Example when we run a program:

```bash
./test
```

Then press `Ctrl+C`, the terminal does not directly call any function in your C program. It causes the process to receive: `SIGINT`

The kernel then handles this signal.

Default behavior:

>SIGINT -> terminate process

## A signal has three important states.

A signal for a process can be viewed in terms of these stages:

```text
            Generated
                | 
                v
            Pending
                | 
                v
            Delivered
```

A crucial point:

>"Generated" does not mean the handler executes immediately.

A signal can be blocked, in which case it becomes pending.

## Signal Disposition

Every signal has a disposition—that is, the action the process takes when it receives that signal.

There are three main types:

```text
        signal
            | 
            +---- default action
            | 
            +---- ignore
            | 
            +---- custom handler
```

If we `ignore` or write a signal handler, we must add it to the signal.

Example we add `handler` function to signal `SIGINT`:

```c
signal(SIGINT, handler);
```

Then we press `Ctrl+C`, the program will not terminate but run the `handler` function.

Model:

```text
                    Kernel
                      |
                 SIGINT generated
                      |
                      v
              +---------------+
              | process signal |
              | disposition    |
              +---------------+
                      |
                      v
                   handler()
```

## Unreliable Signals

This is an important part of the history of Unix signals.

Traditional signals once operated under semantics known as "unreliable signals."

The basic idea:

> Signals might not be preserved as expected if multiple signals occur while the handler is currently processing.

If a signal is in a pending state and other similar signals arrive, they will not be queued like messages in a message queue.

```text
             SIGINT
                |
                v
             pending

             SIGINT
                |
                v
           already pending

             SIGINT
                |
                v
          already pending
```

## Interrupted System Calls

If a process is blocked while making a system call and a signal is sent to it, the blocked state will be interrupted, and the function will return an error code of -1 (if applicable).

Example:

```c
void signal_handler(int sig)
{
    printf("signal SIGINT have received\n");
}

int main(int argc, char *argv[])
{
    signal(SIGINT, signal_handler);

    printf("process is running\n");

    char buf[256];

    int ret = read(STDIN_FILENO, buf, sizeof(buf));

    if (ret == -1)
    {
        printf("read failed\n");
    }

    return 0;
}
```

```text
process is running
^Csignal SIGINT have received
```

When the process is running, we press `Ctrl+C` to send signal `SIGINT` to the process. The block state when `read` is interrupted.

## sigaction()

> NOTE:  
> A signal can interrupt a system call. The subsequent behavior depends on:
>
> - the type of system call
> - the signal disposition
> - SA_RESTART
> - kernel/libc semantics

Example:

```c
struct sigaction sa;

sa.sa_handler = handler;
sigemptyset(&sa.sa_mask);
sa.sa_flags = SA_RESTART;

sigaction(SIGUSR1, &sa, NULL);
```

`SA_RESTART` requests some interrupted system calls auto restart.

## Reentrant Functions

> A function is reentrant if it can be called again before a previous invocation has completed and still operate correctly.

Not all functions are safe to use within a signal handler.

A crucial rule:

> Only call functions defined by POSIX as async-signal-safe within a signal handler.  
> Example:
>
> - write()
> - _exit()
> - kill()
> - signal()

Note: `printf()` is very easy to use in ***demos*** but is not a safe choice for production signal handlers.

**Why is printf() dangerous?**

If a signal occurs while a process is writing data to the `stdout` buffer, the write operation is interrupted; if the signal handler subsequently calls `printf()`, the buffer may be overwritten, resulting in unintended output.

## volatile sig_atomic_t

When a signal occurs, the handler function simply needs to set a `flag` variable to 1 to notify the main program for processing, rather than performing complex tasks within the function itself.

Example:

```c
volatile sig_atomic_t flag = 0;

void handler(int sig)
{
    flag = 1;
}

int main()
{
    while (flag)
    {
        /* do work */
        flag = 0;
    }
}
```

## Reliable Signals

### kill() / raise()

```c
kill(pid, signal);
```

Send `signal` to the process with PID = pid.

|pid||Meaning|
|---|---|---|
|> 0||Send to the specific process|
|0||Send to the current process group|
|< -1||Send to process group with PGID|
|-1||Sent to all processes that the caller has permission to signal|

**Compare `kill()` with `raise()`:**

| | `kill()`             | `raise()`                 |
| ---------------- | -------------------- | ------------------------- |
| Sent to          | process/group        | the process itself        |
| Uses PID         | Yes                  | No                        |
| Inter-process IPC| Yes                  | Not the primary purpose   |
| Example          | `kill(pid, SIGUSR1)` | `raise(SIGUSR1)`          |

### alarm() / pause()

**alarm**

> alarm() : Timer using signals.

```c
#include <unistd.h>

unsigned int alarm(unsigned int seconds);
/* The return value is the number of seconds remaining
                 for the previous alarm, if any. */

/* Example */
alarm(5);

/* After approximately 5 seconds,
 * the process will receive a `SIGALRM`.
 */
```

> alarm() has only one timer.

```c
/* if before */
alarm(10);
/* then */
alarm(3);
/* The 10-second timer was replaced by a 3-second timer. */
```

> Use `alarm(0)` to cancel the current alarm.

**pause**

> `pause()` puts the process to sleep until a signal is received and its handler is executed.

Prototype:

```c
#include <unistd.h>

int pause(void);
```

**Combine `alarm() + pause()`**

```c
void handler(int sig)
{
    printf("SIGALRM received\n");
}

int main(void)
{
    signal(SIGALRM, handler);

    alarm(5);

    printf("waiting...\n");

    pause();

    printf("done\n");

    return 0;
}
```

## Signal Set

```c
sigset_t set;
```

The main API:

|API||Effect|
|---|---|---|
|`sigemptyset()`||Create an empty signal set|
|`sigfillset()`||Add all signals to the set|
|`sigaddset()`||Add another signal|
|`sigdelset()`||Delete signal|
|`sigismember()`||Check if the signal is in the set|

Exxample:

```c
sigset_t set;

sigemptyset(&set); /* set = {} */
sigfillset(&set); /* set = { SIGHUP,
                             SIGINT,
                             SIGQUIT,
                             ...
                             } */
sigaddset(&set, SIGUSR1); /* add SIGUSR1 */
sigaddset(&set, SIGUSR2); /* add SIGUSR2 */
sigdelset(&set, SIGUSR1); /* delete SIGUSR1 */

int ret = sigismember(&set, SIGUSR1);
/* Return: 1 -> yes
           0 -> no
           -1 -> error */
```

### sigprocmask()

> It is used to change the signal mask of a process or thread.

Prototype:

```c
int sigprocmask(int how,
                const sigset_t *set,
                sigset_t *oldset);

/* returns 0 on success.  On failure, -1 is returned
       and errno is set to indicate the error. */
```

`how` has three important values:

|value||mean|
|---|---|---|
|SIG_BLOCK||The set of blocked signals is the union of the current set and the set argument|
|SIG_UNBLOCK||The signals in set are removed from the current set of blocked signals|
|SIG_SETMASK||Replace the entire current mask with the set|

`sigpending()` : It retrieves the set of pending signals.

### sigaction()

Prototype:

```c
int sigaction(
    int signum,
    const struct sigaction *act,
    struct sigaction *oldact
);
```

Structure:

```c
struct sigaction {
    void     (*sa_handler)(int);
    sigset_t   sa_mask;
    int        sa_flags;
    void     (*sa_sigaction)(int, siginfo_t *, void *);
};
```

**Important flags**

| Flag           | Meaning                                                              |
| -------------- | -------------------------------------------------------------------- |
| `SA_RESTART`   | Automatically restart certain system calls interrupted by a signal   |
| `SA_NODEFER`   | Do not automatically block the signal currently being handled        |
| `SA_RESETHAND` | Reset the signal disposition to default when the handler starts      |
| `SA_SIGINFO`   | Use `sa_sigaction` instead of `sa_handler` to receive extra signal info |
| `SA_NOCLDSTOP` | For `SIGCHLD`: do not receive a signal when a child is merely stopped |
| `SA_NOCLDWAIT` | For `SIGCHLD`: prevent the child from becoming a zombie              |

### sigsuspend()

> Temporarily replaces the signal mask of the calling thread with the mask given by mask and then suspends the thread until delivery of a signal whose action is to invoke a signal handler or to terminate a process.

When we call `sigsuspend()`, kernel atomically transitions the state:

```text
                OLD MASK
                ────────────
                SIGUSR1 BLOCKED

                        ↓ sigsuspend()

                TEMP MASK
                ────────────
                SIGUSR1 UNBLOCKED
```

Example, first we block signal `SIGUSR1` and then we use `sigsuspend()` to temporarily unlock this signal, the process operates as follow:

```text
                          Process
                             │
                      SIGUSR1 BLOCKED
                             │`
                             ▼
                         running code
                             |
                             ▼
                        sigsuspend()
                             │
                             ▼
                    ┌─────────────────┐
                    │ SIGUSR1 UNBLOCK │
                    │                 │
                    │      SLEEP      │
                    └────────┬────────┘
                             │
                             │ SIGUSR1
                             ▼
                         handler()
                             │
                             ▼
                     handler return
                             │
                             ▼
                     restore OLD MASK
                             │
                             ▼
                   sigsuspend() returns
```
