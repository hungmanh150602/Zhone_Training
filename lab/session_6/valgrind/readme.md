# In this example, I will continue using ChatGPT to create a program that has memory leak and a use-after-free bug, and then debug it using `valgrind`.

>I don't know anything about it.

I compile and run code. The ressult as below:

```text
p1 = 10
p2 = 1382864013
```

Run the program with `valgrind`:

```bash
valgrind ./valgrind 
```

```text
==57423== Memcheck, a memory error detector
==57423== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==57423== Using Valgrind-3.18.1 and LibVEX; rerun with -h for copyright info
==57423== Command: ./valgrind
==57423== 
p1 = 10
==57423== Invalid read of size 4
==57423==    at 0x1091F1: main (in /home/hungubuntu/Vim_C_code/a_lab/session_6/valgrind/valgrind)
==57423==  Address 0x4ab04d0 is 0 bytes inside a block of size 4 free'd
==57423==    at 0x484B27F: free (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==57423==    by 0x1091EC: main (in /home/hungubuntu/Vim_C_code/a_lab/session_6/valgrind/valgrind)
==57423==  Block was alloc'd at
==57423==    at 0x4848899: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==57423==    by 0x1091D2: main (in /home/hungubuntu/Vim_C_code/a_lab/session_6/valgrind/valgrind)
==57423== 
p2 = 20
==57423== 
==57423== HEAP SUMMARY:
==57423==     in use at exit: 4 bytes in 1 blocks
==57423==   total heap usage: 3 allocs, 2 frees, 1,032 bytes allocated
==57423== 
==57423== LEAK SUMMARY:
==57423==    definitely lost: 4 bytes in 1 blocks
==57423==    indirectly lost: 0 bytes in 0 blocks
==57423==      possibly lost: 0 bytes in 0 blocks
==57423==    still reachable: 0 bytes in 0 blocks
==57423==         suppressed: 0 bytes in 0 blocks
==57423== Rerun with --leak-check=full to see details of leaked memory
==57423== 
==57423== For lists of detected and suppressed errors, rerun with: -s
==57423== ERROR SUMMARY: 1 errors from 1 contexts (suppressed: 0 from 0)
```

As the results show, I have an error `Invalid read of size 4` in function `main` due to use after `free` and another error `in use at exit: 4 bytes in 1 block` meaning the program ends but the memory is still in use and has not been freed (`definitely lost: 4 bytes in 1 block`).