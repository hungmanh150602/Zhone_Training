#include <stdio.h>

int add(int a, int b);

extern int x;

int main(void)
{
    printf("%d", add(x,20));
    return add(10,20);
}