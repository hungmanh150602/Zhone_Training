#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
   setpgid();
   printf("Before system.\n");
   system("echo Hello");
   system("ls -l");
   printf("After system");
   // system("ls *.c");
   return 0;
}