# I request the ChatGPT AI to generate a program that have *Segmentation fault* error. Then I use *gdb* to check and find the location of error.

First, when running the program, the result is below:

```text
Starting application...

========== DATABASE ==========
ID: 1 | Name: User_1 | Score: 10
ID: 2 | Name: User_2 | Score: 20
ID: 3 | Name: User_3 | Score: 30
ID: 4 | Name: User_4 | Score: 40
ID: 5 | Name: User_5 | Score: 50
==============================

Searching for user 999...
User found. Updating score...
Segmentation fault (core dumped)
```

We will ignore the initial lines and focus on the last one: `Segmentation fault (core dumped)`. It appears the program stopped somewhere along the way.

# 1. Debug live with GDB

The first step in debugging when I have the source code and am running the program with gdb:

```bash
gdb ./test
```

After opening the gdb interface, I set a breakpoint at the main function to halt the program there:

```gdb
(gdb) break main
Breakpoint 1 at 0x1545: file test_debug.c, line 140.
```

Type the `run` command to let the program continue running:

```gdb
(gdb) run
Starting program: /home/hungubuntu/Vim_C_code/session_6/test_debug 
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".

Breakpoint 1, main () at test_debug.c:140
140	    printf("Starting application...\n");
```

Okay, the program has paused at `main`. Of course, there is nothing to inspect yet because the program hasn't executed any lines within the `main` function.

To proceed to the next instruction, I use the `next` command.

```gdb
(gdb) next 
Starting application...
142	    db = database_create(MAX_USERS);
(gdb) 
144	    if (db == NULL) {
(gdb) 
149	    initialize_users(db);
```

After I run the `next` command three times, I now have a `db` variable that I can inspect. I will print its value using `print db`:

```gdb
(gdb) print db
$1 = (Database *) 0x5555555596b0
```

Oh, it looks like this is a pointer; I'll proceed to examine the value at the address it points to by using `print *db`:

```gdb
(gdb) print *db
$2 = {users = 0x5555555596d0, count = 5}
```

Everything looks perfectly normal; there are no errors here. I proceed with `next` to check the next command.

```gdb
158	    process_user(db, 999);
(gdb) 

Searching for user 999...
User found. Updating score...

Program received signal SIGSEGV, Segmentation fault.
0x0000555555555401 in increase_score (user=0x0, amount=5) at test_debug.c:90
90	    user->score += amount;
```

When I reached line 158—`process_user(db, 999);`—and stepped over it, an error occurred, indicating that line 90 was causing the program to crash.

I had to restart the program since it had previously terminated due to the error. Now that I know where the issue lies, I’ll set another breakpoint at the function causing the error to see exactly what is happening.

```gdb
Breakpoint 2, process_user (db=0x5555555596b0, id=999) at test_debug.c:98
98	    printf("\nSearching for user %d...\n", id);
```

The program has paused here, and I'll check the local variables:
`info locals`:

```gdb
(gdb) info locals 
user = 0x5555596b0
(gdb) info  args 
db = 0x5555555596b0
id = 999
(gdb) print *db
$3 = {users = 0x5555555596d0, count = 5}
```

I’ve gone ahead and checked both the local variables and the passed parameters. I noticed two values ​​with a significant discrepancy: `id = 999` and `db.count = 5`. Let’s check what happens `next`:

```gdb
(gdb) next 

Searching for user 999...
100	    user = find_user(db, id);
(gdb) 
102	    printf("User found. Updating score...\n");
(gdb) 
User found. Updating score...
104	    increase_score(user, 5);
(gdb) print user
$4 = (User *) 0x0
```

Oh, I noticed that after executing a few commands, the `user` pointer was pointing to `0x0`—a value representing a `NULL` pointer—and line 90 in the function `increase_score(user, 5)` (`user->score += amount;`) caused the program to crash.

At this point, I can confirm that the crash occurred because we attempted to write to a memory location (in this case, `NULL`).

The variable became `user = NULL` after the function `user = find_user(db, id);` was executed. Why did it return `NULL`? Let's open the source code and take a look:

```c
/* Find user by ID */
User *find_user(Database *db, int id)
{
    int i;

    for (i = 0; i < db->count; i++) {
        if (db->users[i].id == id) {
            return &db->users[i];
        }
    }

    return NULL;
}
```

This is the function that searches for a user by `id`. It is called within `process_user(db, 999)`; upon inspecting the variables inside this function, I found that `id` is 999 and `db.count` is 5.

By cross-referencing this with the search function, I identified the cause: we are searching for a user who is not in the list. Consequently, a `NULL` value is returned, yet we proceed to write data to that memory location without checking whether it points to `NULL`.

# 2. Debug with coredump file

When a program crashes and generates a coredump file, we can use that file for debugging.

Run the coredump file with gdb:

```bash
 gdb ./test_debug core.test_debug.8945 
```

```gdb
Program terminated with signal SIGSEGV, Segmentation fault.
#0  0x0000555555555401 in increase_score (user=0x0, amount=5) at test_debug.c:90
90	    user->score += amount;
```

The core dump file allows us to determine exactly which line caused the program to stop. I will then use `backtrace` to locate that line and see which functions called it:

```gdb
(gdb) backtrace 
#0  0x0000555555555401 in increase_score (user=0x0, amount=5) at test_debug.c:90
#1  0x0000555555555474 in process_user (db=0x5555555596b0, id=999) at test_debug.c:104
#2  0x00005555555555bc in main () at test_debug.c:158
```

We have a `main` function that calls `process_user`, and `process_user` in turn calls `increase_score`.

I'm taking a look at the code surrounding the line where the error occurred:

```gdb
(gdb) list 90
85	}
86	
87	/* Increase user's score */
88	void increase_score(User *user, int amount)
89	{
90	    user->score += amount;
91	}
92	
93	/* Process user */
94	void process_user(Database *db, int id)
```

The function consists of just a single line of code and is nothing special; I'm checking the value of the `user` variable:

```gdb
(gdb) print user
$1 = (User *) 0x0
```

The `user` variable points to `NULL`; it appears to have been assigned `NULL` before being passed to the `increase_score` function. Let's go back to the calling function and investigate further—remember, the command to switch frames is `frame 1` (switch to frame 1).

```gdb
(gdb) frame 1
#1  0x0000555555555474 in process_user (db=0x5555555596b0, id=999) at test_debug.c:104
104	    increase_score(user, 5);
```

We are now at the `process_user` function; looking at the source code, I can see that the pointer variable `user` is assigned the result of the `find_user(db, id)` function, and its value is `NULL`.

Let's check the values ​​of the other variables:

```gdb
(gdb) info args 
db = 0x5555555596b0
id = 999
(gdb) print *db
$2 = {users = 0x5555555596d0, count = 5}
```

We have `id = 999` and `db.count = 5` passed into the `find_user` function. Let's examine the function's source code to see what it does:

```c
/* Find user by ID */
User *find_user(Database *db, int id)
{
    int i;

    for (i = 0; i < db->count; i++) {
        if (db->users[i].id == id) {
            return &db->users[i];
        }
    }

    return NULL;
}
```

It can be concluded that the function did not find a user with a matching `id`, so it returned `NULL`. The issue has been identified.
