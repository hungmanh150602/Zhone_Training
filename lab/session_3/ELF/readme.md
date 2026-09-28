# This example use `nm/readelf/ldd/objdump` to inspect a self-written binary

## 1. compile file main.c to object file

```bash
gcc main.c -o main
```

## 2. use `nm` to view symbol table:

```bash
nm main
```

We can see:

```text
0000000000001149 t add
0000000000004010 D global
0000000000001183 T main
                 U printf@GLIBC_2.2.5
0000000000004018 b static_x
```

I will explain an example:

```text
0000000000004010         D           global
        |                |             |
     address         symbol type     name
```

Some symbol types:

- t: text/code, local
- T: text/code, global
- D: initialized data
- d: initialized data, local
- B: BSS
- b: BSS, local
- U: undefined
- R: read-only data

## 3. use `readelf` to view ELF header

```bash
readelf -h main
```

```text
ELF Header:
  Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              DYN (Position-Independent Executable file)
  Machine:                           Advanced Micro Devices X86-64
  Version:                           0x1
  Entry point address:               0x1060
  Start of program headers:          64 (bytes into file)
  Start of section headers:          14080 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         13
  Size of section headers:           64 (bytes)
  Number of section headers:         31
  Section header string table index: 30
```

## 4. use `readelf -S main` to view Sections

```bash
readelf -S main
```

```text
There are 31 section headers, starting at offset 0x3700:

Section Headers:
  [Nr] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
  [16] .text             PROGBITS         0000000000001060  00001060
       000000000000014b  0000000000000000  AX       0     0     16
  [18] .rodata           PROGBITS         0000000000002000  00002000
       0000000000000008  0000000000000000   A       0     0     4
  [25] .data             PROGBITS         0000000000004000  00003000
       0000000000000014  0000000000000000  WA       0     0     8
  [26] .bss              NOBITS           0000000000004014  00003014
       000000000000000c  0000000000000000  WA       0     0     4
  [27] .comment          PROGBITS         0000000000000000  00003014
       000000000000002d  0000000000000001  MS       0     0     1
  [28] .symtab           SYMTAB           0000000000000000  00003048
       00000000000003a8  0000000000000018          29    20     8
  [29] .strtab           STRTAB           0000000000000000  000033f0
       00000000000001f0  0000000000000000           0     0     1
  [30] .shstrtab         STRTAB           0000000000000000  000035e0
       000000000000011a  0000000000000000           0     0     1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  D (mbind), l (large), p (processor specific)
```

## 5. use `readelf -s main` to view Symbols table

```bash
readelf -s main
```

```text
Symbol table '.symtab' contains 39 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
    11: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS main.c
    12: 0000000000004018     4 OBJECT  LOCAL  DEFAULT   26 static_x
    13: 0000000000001149    58 FUNC    LOCAL  DEFAULT   16 add
    25: 0000000000004010     4 OBJECT  GLOBAL DEFAULT   25 global
    26: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND printf@GLIBC_2.2.5
    34: 0000000000001183    40 FUNC    GLOBAL DEFAULT   16 main
```

## 6. use `ldd` to view depended library

```bash
ldd main
```

```text
	linux-vdso.so.1 (0x00007ffdb79e1000)
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x000072cd95200000)
	/lib64/ld-linux-x86-64.so.2 (0x000072cd954ef000)
```

## 7. use `objdump` to view machine code

```bash
objdump -d main
```

```text
0000000000001149 <add>:
    1149:	f3 0f 1e fa          	endbr64 
    114d:	55                   	push   rbp
    114e:	48 89 e5             	mov    rbp,rsp
    1151:	48 83 ec 10          	sub    rsp,0x10
    1155:	89 7d fc             	mov    DWORD PTR [rbp-0x4],edi
    1158:	89 75 f8             	mov    DWORD PTR [rbp-0x8],esi
    115b:	8b 55 fc             	mov    edx,DWORD PTR [rbp-0x4]
    115e:	8b 45 f8             	mov    eax,DWORD PTR [rbp-0x8]
    1161:	01 d0                	add    eax,edx
    1163:	89 c6                	mov    esi,eax
    1165:	48 8d 05 98 0e 00 00 	lea    rax,[rip+0xe98]        # 2004 <_IO_stdin_used+0x4>
    116c:	48 89 c7             	mov    rdi,rax
    116f:	b8 00 00 00 00       	mov    eax,0x0
    1174:	e8 d7 fe ff ff       	call   1050 <printf@plt>
    1179:	8b 55 fc             	mov    edx,DWORD PTR [rbp-0x4]
    117c:	8b 45 f8             	mov    eax,DWORD PTR [rbp-0x8]
    117f:	01 d0                	add    eax,edx
    1181:	c9                   	leave  
    1182:	c3                   	ret    

0000000000001183 <main>:
    1183:	f3 0f 1e fa          	endbr64 
    1187:	55                   	push   rbp
    1188:	48 89 e5             	mov    rbp,rsp
    118b:	48 83 ec 10          	sub    rsp,0x10
    118f:	8b 05 7b 2e 00 00    	mov    eax,DWORD PTR [rip+0x2e7b]        # 4010 <global>
    1195:	89 c6                	mov    esi,eax
    1197:	bf 0c 00 00 00       	mov    edi,0xc
    119c:	e8 a8 ff ff ff       	call   1149 <add>
    11a1:	89 45 fc             	mov    DWORD PTR [rbp-0x4],eax
    11a4:	b8 00 00 00 00       	mov    eax,0x0
    11a9:	c9                   	leave  
    11aa:	c3                   	ret 
```