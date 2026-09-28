# The purpose of this example is observe zombie process.

First i will create child process and then, i let the child process die first to turn it into a zombie.

when run this code, i will receive:

```text
Parent PID = 160287
Child PID = 160288
Parent sleeps for 30 seconds...
Child PID = 160288
Child exits now
```

Child will die before parent die.
We can use command `ps -aux` to observe zombie process.

```bash
USER         PID %CPU %MEM    VSZ   RSS TTY      STAT START   TIME COMMAND

hungubu+  160287  0.0  0.0   2776  1592 pts/0    S+   14:40   0:00 ./test
hungubu+  160288  0.0  0.0      0     0 pts/0    Z+   14:40   0:00 [test] <defunct>
```

We can see the STAT field is Z+, this is zombie process.
