# 1. Introduction to Cross-Compilation

## 1.1 What is cross-compilation?

> **Cross-compilation** is the process of compiling software on one system to produce executable code for a different target system.

Example we want to build a program on our laptop and run it on Raspberry Pi. Consider our environment:

| Property | Development machine | Target device |
|---|---|---|
| Hardware | Desktop or laptop | Raspberry Pi 4 |
| Operating system | Ubuntu | Raspberry Pi OS |
| CPU architecture | x86-64 | AArch64 |
| Purpose | Compile and build software | Execute the resulting program |

**Native compilation**

> Native compilation means compiling software for the system on which you're working.

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

## 1.2 How does cross-compilation work?

**Three terms to remember**

GCC and build systems use the terms build, host, and target. Their meanings depend on which software you're building.

| Term | Meaning | Example |
|---|---|---|
| Build | The system that performs the build process | Ubuntu x86-64 |
| Host | The system that runs the program that has been built (where GCC will run) | Raspberry Pi |
| Target | The system for which the tool is designed to generate code (where the program compiled by GCC will run) | ARM64 (in a typical scenario) |

Example when we config `crosstool-ng`:

```text
[EXTRA]  Dumping internal crosstool-NG configuration
[EXTRA]    Building a toolchain for:
[EXTRA]      build  = x86_64-pc-linux-gnu
[EXTRA]      host   = x86_64-pc-linux-gnu
[EXTRA]      target = aarch64-unknown-linux-gnu
```

![alt text](image.png)

**There are four cases of build-host-target:**

1. *build == host == target (“native”)*

> This is a plain native toolchain, targeting the exact same machine as the one it is built on, and running again on this exact same machine. 

2. *build == host != target (“cross”)*

> This is a classic cross-toolchain, which is expected to be run on the same machine it is compiled on, and generate code to run on a second machine, the target.

3. *build != host == target (“cross-native”)*

> Such a toolchain is also a native toolchain, as it targets the same machine as it runs on. But it is build on another machine.

4. *build != host != target (“canadian”)*

> This one is called a “Canadian Cross”¹ toolchain, and is tricky. The three machines in play are different. You might want such a toolchain if you have a fast build machine, but the users will use it on another machine, and will produce code to run on a third machine.

## 1.3 What’s inside a Cross-Compilation toolchain?

> A toolchain is a collection of programs, libraries, headers, and supporting files used to transform source code into an executable for a particular target.

A typical C cross-compilation toolchain contains the following components.

| Component | Function | Example |
|---|---|---|
| GCC | Compiles C/C++ code into machine code | `aarch64-linux-gnu-gcc` |
| Binutils | Handles object files, libraries, and executables | `as`, `ld`, `ar`, `objdump` |
| C library | Provides standard functions and a userspace interface to Linux | glibc, musl |
| Linux kernel headers | Provide definitions required to build userspace software that interacts with the kernel | UAPI headers |
| Sysroot | A directory tree containing headers and libraries for the target | `sysroot/usr/include`, `sysroot/usr/lib` |
| GDB | Debugs programs | `aarch64-linux-gnu-gdb` |

**What does the GCC do?**

> GCC generates code for the target architecture even though GCC itself executes on your x86-64 Ubuntu machine.

**GNU Binutils — assembler, linker, and binary utilities**

> GNU Binutils supplies tools used to construct and inspect executable files.

Important components include:

| Tool | Purpose |
|---|---|
| `as` | Assembles target assembly code |
| `ld` | Links object files and libraries |
| `ar` | Creates and manages static archives |
| `nm` | Lists symbols in object files and executables |
| `objdump` | Disassembles and examines binary files |
| `readelf` | Inspects ELF executable metadata |

Example:

```bash
aarch64-linux-gnu-as
aarch64-linux-gnu-ld
aarch64-linux-gnu-ar
aarch64-linux-gnu-nm
aarch64-linux-gnu-objdump
aarch64-linux-gnu-readelf
```

**What is a sysroot?**

> A sysroot is a directory which is considered to be the root directory for the purpose of locating headers and libraries.

Suppose your application uses a function declared in a target header and implemented by the target C library.

The compilation process needs both:

1. The appropriate header declaration.
2. The appropriate library implementation at link time.

If the compiler uses host headers or the linker selects incompatible libraries, the build might fail or produce an executable that cannot run correctly on the target.

When building your own toolchain with Crosstool-NG, we'll configure the target system and its libraries so the compiler has a consistent environment.

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

Typical characteristics:

- Suitable for many lightweight Linux environments.

- Used by Alpine Linux.

- Provides an alternative to glibc.

- May require adjustments when porting software that depends on glibc-specific behavior or interfaces.

A program built for musl should not be assumed to run on a system that only provides glibc, or vice versa.

> **Best use case**: Embedded systems, minimal Linux distributions, and environments where you control the runtime and want to use musl deliberately.

## 2.3 uClibc

> `uClibc` is a C library aimed at embedded Linux systems.

Typical characteristics:

- Designed with embedded use cases in mind.

- Can be configured to suit particular system requirements.

- May be useful in constrained environments.

- Requires checking application compatibility and the availability of maintained toolchain components.

> **Best use case**: Specialized embedded Linux systems where the developer controls the software stack and has a specific reason to select uClibc.

**Comparison**

| Criterion | glibc | musl | uClibc-ng |
|---|---|---|---|
| Typical environment | General-purpose Linux | Lightweight Linux | Embedded Linux |
| Compatibility with mainstream glibc applications | Native fit for compatible versions | May require porting | May require porting |
| Suitable for embedded devices | Yes | Yes | Yes |
| Runtime compatibility across different libc implementations | Not guaranteed | Not guaranteed | Not guaranteed |
| Recommended for our initial Pi 4 exercise | **Yes** | Not initially | Not initially |

# 3. Ready-to-use cross-compiling toolchains vs. building a custom toolchain

## What is a prebuilt toolchain?

> A prebuilt toolchain is a collection of tools that someone has already compiled and packaged for you.

Example:

```bash
sudo apt install gcc-aarch64-linux-gnu
```

**Advantages**

- Fast to install.

- No need to compile GCC and glibc yourself.

- Suitable for learning and everyday development.

- Often integrates well with the development distribution.

**Disadvantages**

- You have less control over the exact component versions.

- Its default configuration might not match a specialized target.

- You may need to provide a separate target sysroot to build applications against the target's actual libraries.

## What is a custom toolchain?

> A custom toolchain is one you build with explicitly selected configuration options.

For example, you may choose:

| Configuration | Example choice |
|---|---|
| Target architecture | AArch64 |
| Operating system | Linux |
| C library | glibc |
| GCC version | A compatible selected release |
| Binutils version | A compatible selected release |
| Kernel headers | A suitable Linux UAPI header version |
| Installation directory | `~/x-tools/` |

**Advantages**

- Greater control over compiler and library versions.

- A configuration that can be saved and reproduced.

- Useful for maintaining a stable embedded development environment.

- Helpful when building for a custom Linux distribution.

**Disadvantages**

- Requires more initial setup.

- Can take substantial time and disk space.

- Version incompatibilities can cause build failures.

- You must understand the target's runtime requirements.

## Comparison

| Criterion | Prebuilt toolchain | Custom toolchain |
|---|---|---|
| Setup time | Short | Longer |
| Component selection | Limited by the package | Explicitly configurable |
| Learning value | Good for learning cross-compilation | Good for learning toolchain construction |
| Reproducibility | Depends on package and version management | Can be improved by saving configuration and source versions |
| Typical use | Application development and quick experiments | Specialized embedded development and controlled build environments |

# 4. Introduction to Crosstool-NG

## 4.1 What is Crosstool-NG?

> Crosstool-NG is a toolchain generator that automates the process of building cross-compilation toolchains.

Without an automated tool, you would need to coordinate the configuration and compilation of multiple components, including GCC, Binutils, the C library, and target development headers.

Crosstool-NG manages much of this process through a configurable build system.

## 4.2 How do we configure Crosstool-NG?

The basic workflow:

```text
       Development machine
           Ubuntu x86-64
                 |
                 v
       Install Crosstool-NG
                 |
                 v
       Select a sample config
                 |
                 v
         Configure the target
                 |
                 v
            ct-ng build
                 |
                 v
      Download source components
                 |
                 v
     Build GCC, libc, and Binutils
                 |
                 v
       Install the cross-toolchain
                 |
                 v
       Cross-compile applications
                 |
                 v
         Run them on the Pi
```

## 4.3 Building a Custom Cross-Compilation Toolchain

# Reference materials

Read these in order:

1. Crosstool-NG: Introduction — Why toolchains exist and what they contain.  
<https://crosstool-ng.github.io/docs/introduction/?utm_source=chatgpt.com>
2. Crosstool-NG: Toolchain Types — Build, host, target, and different toolchain configurations.  
<https://crosstool-ng.github.io/docs/toolchain-types/?utm_source=chatgpt.com>
3. GCC: Configuration Terminology — How GCC defines build, host, and target.  
<https://gcc.gnu.org/onlinedocs/gcc-7.1.0/gccint/Configure-Terms.html?utm_source=chatgpt.com>
4. Crosstool-NG: Configuration — How to customize a toolchain.  
<https://crosstool-ng.github.io/docs/configuration/?utm_source=chatgpt.com>
5. Crosstool-NG: Building the Toolchain — How the configured toolchain is built.  
<https://crosstool-ng.github.io/docs/configuration/?utm_source=chatgpt.com>
