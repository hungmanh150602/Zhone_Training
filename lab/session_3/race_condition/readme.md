# The purpose of this example is reproduce a race condition between parent and child when both of them access the same memory region.

I will create child process, then both parent and child process will increate the value of shared memory to 100000 times.  
I expected a result of 200000, but in reality, I received a lower value.

The result after i run this code:

```text
Final counter = 100463
```

This is **race condition**.
