# The purpose of this example is useing command in terminal to observe the process group/session of the program

Command:

```bash
ps -p 20574 -o pid,ppid,pgid,sid,cmd
```

First we can run this example and see the output as below:

```text
parrent process:
PID: 143140
PPID: 10229
PGID: 143140
SID: 10229
---------------------------
child process:
PID: 143141
PPID: 143140
PGID: 143140
SID: 10229
---------------------------
```

Then we use command to observe the process group/session of the program

```text
    PID    PPID    PGID     SID CMD
 143140   10229  143140   10229 ./test
 143141  143140  143140   10229 [test] <defunct>

```
