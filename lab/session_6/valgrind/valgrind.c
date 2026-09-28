#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    int *p1;
    int *p2;

    p1 = malloc(sizeof(int));
    *p1 = 10;
    printf("p1 = %d\n", *p1);

    p2 = malloc(sizeof(int));
    *p2 = 20;

    free(p2);

    printf("p2 = %d\n", *p2);

    return 0;
}