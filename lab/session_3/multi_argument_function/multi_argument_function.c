#include <stdio.h>

static int add(int a, int b, int c, int d, int e, int f,
               int g, int h)
{
    return a + b + c;
}

int main(void)
{
    int result = add(1, 2, 3, 4, 5, 6, 7, 8);
    printf("%d", result);
    return 0;
}