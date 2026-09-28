#include <stdio.h>
#include <stdlib.h>

int global_init = 10;
int global_uninit;

static int static_init = 20;
static int static_uninit;

/*
function print out address of:
    foo function
    local variable in function
*/
void foo(void)
{
    int local = 30;

    printf("foo:             %p\n", (void *)foo);
    printf("local:           %p\n", (void *)&local);
}

int main(void)
{
    int local = 40;
    int *heap = malloc(sizeof(int));

    *heap = 50;

    printf("main:            %p\n", (void *)main);

    printf("global_init:     %p\n", (void *)&global_init);
    printf("global_uninit:   %p\n", (void *)&global_uninit);

    printf("static_init:     %p\n", (void *)&static_init);
    printf("static_uninit:   %p\n", (void *)&static_uninit);

    printf("local:           %p\n", (void *)&local);

    printf("heap:            %p\n", (void *)heap);

    foo();

    getchar();
    free(heap);

    return 0;
}