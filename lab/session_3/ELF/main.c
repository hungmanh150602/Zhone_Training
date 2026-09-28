#include <stdio.h>

#define SQUARE(x) ((x) * (x))

int global = 100;
static int static_x;
extern int extern_x;

static int add(int a, int b)
{
    printf("%d\n", a+b);
    return a + b;
}

int main(void)
{
    int x = add(12, global);

    return 0;
}