# 1. Introduction to Cross-Compilation

## What is cross-compilation?

> **Cross-compilation** is the process of compiling software on one system to produce executable code for a different target system.

Example we want to build a program on our laptop and run it on Raspberry Pi. Consider our environment:

| Property | Development machine | Target device |
|---|---|---|
| Hardware | Desktop or laptop | Raspberry Pi 4 |
| Operating system | Ubuntu | Raspberry Pi OS |
| CPU architecture | x86-64 | AArch64 |
| Purpose | Compile and build software | Execute the resulting program |

**Native compilation**

> Native compilation means compiling software for the system on which the resulting program is intended to run

Example:

```bash
gcc hello.c -o hello
```

The resulting executable will normally target the native x86-64 Linux environment.

**Cross-compilation**

> Cross-compilation is the process of compiling for a system different from the one on which the build is being performed.

Example:

```bash
aarch64-linux-gnu-gcc hello.c -o hello_arm64
```

The compiler runs on Ubuntu, but the resulting executable targets AArch64 Linux.

## How does cross-compilation work?

![alt text](image.png)

# 2. Three main choices for the C library

## 2.1 glibc

> `glibc` stands for GNU C Library.

It is widely used by general-purpose Linux distributions, including Debian and Ubuntu.

Typical characteristics:

- Broad compatibility with software built for mainstream GNU/Linux environments.

- Extensive support for C and POSIX interfaces.

- A mature ecosystem.

- Compatibility requirements that depend on the glibc version and other runtime dependencies.

For example, a dynamically linked program might require GLIBC_2.xx symbols. If the target system provides an older glibc version, the program may fail to start.

> **Best use case**: General-purpose Linux applications where compatibility with an existing glibc-based operating system matters.

## 2.2 musl

> `musl` is a C library designed around simplicity, standards conformance, and a relatively compact implementation.

## 2.3 uClibc-ng
