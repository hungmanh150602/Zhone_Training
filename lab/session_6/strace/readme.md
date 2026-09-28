# In this example, I will continue using ChatGPT to create a program that hangs, and then debug it using `strace`.

Run the program with `strace`:

```bash
strace ./strace
```

```text
openat(AT_FDCWD, "/root/strace_lab.log", O_WRONLY|O_CREAT|O_APPEND, 0666) = -1 EACCES (Permission denied)
socket(AF_INET, SOCK_STREAM, IPPROTO_IP) = 3
bind(3, {sa_family=AF_INET, sin_port=htons(5555), sin_addr=inet_addr("0.0.0.0")}, 16) = 0
listen(3, 5)                            = 0
accept(3, NULL, NULL
```

Looking at the last few lines, I notice the program is paused at the `accept` function; seeing the `bind` and `listen` functions as well, it appears to be a program that creates a socket to listen for and connect to other sockets.

It is clear that the `accept` function causes the program to block when there are no incoming socket connections.
