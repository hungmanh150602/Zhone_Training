# test_compile
# Stage 1 — Preprocessor: gcc -E
```c
gcc -E test_compile.c -o test_compile.i
```

Origin program:  
<img width="97" height="102" alt="image" src="https://github.com/user-attachments/assets/e480600f-c770-4c75-8437-c9d7ef788460" />
```c
#define SQUARE(x) ((x) * (x))

int add(int a, int b)
{
    return a + b;
}

int main(void)
{
    int x = 10;
    int y = 20;

    printf("sum = %d\n", add(x, y));
    printf("square = %d\n", SQUARE(x));

    return 0;
}
```
The preprocessed program:  
<img width="97" height="102" alt="image" src="https://github.com/user-attachments/assets/8c3908ed-8b01-4e9b-abeb-bdb6fdcfda6d" />
```c
int add(int a, int b)
{
    return a + b;
}

int main(void)
{
    int x = 10;
    int y = 20;

    printf("sum = %d\n", add(x, y));
    printf("square = %d\n", ((x) * (x)));

    return 0;
}
```
# Stage 2 — Compiler: gcc -S
```c
gcc -S test_compile.c -o test_compile.s
```
<img width="97" height="102" alt="image" src="https://github.com/user-attachments/assets/879c3a22-1c99-4f67-b290-dfc44ccb3bf5" />  

Output:
```text
add:
.LFB0:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	%edi, -4(%rbp)
	movl	%esi, -8(%rbp)
	movl	-4(%rbp), %edx
	movl	-8(%rbp), %eax
	addl	%edx, %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	add, .-add
	.section	.rodata
.LC0:
	.string	"sum = %d\n"
.LC1:
	.string	"square = %d\n"
	.text
	.globl	main
	.type	main, @function
```
# Stage 3 — Assembler: gcc -c
```c
gcc -c test_compile.c -o test_compile.o
```
<img width="97" height="102" alt="image" src="https://github.com/user-attachments/assets/367622e5-591b-4591-958d-e274e206e746" />  

# Stage 4 — Linker
```c
gcc test_compile.o -o tset_compile  
```
<img width="97" height="102" alt="image" src="https://github.com/user-attachments/assets/58eefcd4-8944-433c-86b1-503c4f26c809" />
