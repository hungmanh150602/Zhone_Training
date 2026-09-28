# This purpose of example is:

# Compile a multi-argument C function to Assembly with gcc -S and annotate its stack frame: saved %rbp, return address, local variables, and any register-passed arguments

## use *`gcc -S multi_argument_function.c -o multi_argument_function.s`* to compile C file into Assembly file.

main function:

```text
main:
	pushq	%rbp            push rbp pointer onto stack
	movq	%rsp, %rbp      move rsp pointer to the rbp location
    
	subq	$16, %rsp
	pushq	$8              <---- h
	pushq	$7              <---- g
	movl	$6, %r9d        <---- f
	movl	$5, %r8d        <---- e
	movl	$4, %ecx        <---- d
	movl	$3, %edx        <---- c
	movl	$2, %esi        <---- b
	movl	$1, %edi        <---- a
	call	add             <---- call function add
	addq	$16, %rsp
	movl	%eax, -4(%rbp)
	movl	-4(%rbp), %eax
	movl	%eax, %esi
	leaq	.LC0(%rip), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf@PLT
	movl	$0, %eax

	leave
	ret
```

add function:

```text
add:
	pushq	%rbp                    push rbp pointer onto stack
	movq	%rsp, %rbp              move rsp pointer to the rbp location

	movl	%edi, -4(%rbp)          move value of resgistor onto stack
	movl	%esi, -8(%rbp)          move value of resgistor onto stack
	movl	%edx, -12(%rbp)         move value of resgistor onto stack
	movl	%ecx, -16(%rbp)         move value of resgistor onto stack
	movl	%r8d, -20(%rbp)         move value of resgistor onto stack
	movl	%r9d, -24(%rbp)         move value of resgistor onto stack
	movl	-4(%rbp), %edx          |
	movl	-8(%rbp), %eax          |
	addl	%eax, %edx              |  implement add operation
	movl	-12(%rbp), %eax         |
	addl	%edx, %eax              |
	popq	%rbp

	ret                             return value in %eax
```
