# This example I create a program having 1000 thread and each thread increase `x` by 1 and 1000 times.

The resutl I expect is 1000000, but let's look at the actual result below:

>x = 994920

Why? Because this is ***race condition*** when we use multi-thread and shared memory.

Then I use `phtread_mutex` to protect the critical session. and the result is:

>x = 1000000

***race condition*** has been removed.