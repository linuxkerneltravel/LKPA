##### 链接脚本样例
使用 `ld --verbose | sed '$d' | sed '1,13d' > s.lds` 指令导出默认链接脚本如下为 `s.lds` 文件。

**定制脚本 `main.lds`, 差异如下 :**
```
[root@VM-8-2-centos lds_test]# diff s.lds main.lds
72,73c72,73
<   PROVIDE (etext = .);
<   .rodata         : { *(.rodata .rodata.* .gnu.linkonce.r.*) }
---
>   PROVIDE (etext = .);
>   .rodata         : { *(.rodata .rodata.* __node1 __node2 .gnu.linkonce.r.*) }
```
可以看到添加了 `__node1 __node2` 两个 `section name`作为 `.rodata` 的一个 `input section`。

**Makefile :**
```
.PHONY : all clean cleanobj

C =gcc -c $< -o $@
OBJ = main.o node2.o node1.o

all : main

main : main.o node2.o node1.o main.lds
         gcc -lc main.o node2.o node1.o -o $@ -T main.lds

main.o : main.c
        $(C)
node2.o : node2.c
        $(C)
node1.o : node1.c
        $(C)

cleanobj :
        rm -rf $(OBJ)

clean :
        rm -rf $(OBJ) main
```
**define.h :**
```
struct node{
        int size;
};
```
**node1.c :**
```
#include "define.h"

const struct node node1
__attribute__((section("__node1"))) = {
        .size = 1
};
```
**node2.c :**
```
#include "define.h"

const struct node node2
__attribute__((section("__node2"))) = {
        .size = 1
};
```
**main.c :**
```#include <stdio.h>
extern struct node node1, node2;

int main (){
        printf("%d   %d\n",&node1, &node2);
        return 0;
}
```
这几个文件就是通过简化这个 `patch` 写的小例子(将所有文件放在同一个文件夹即可)。
使用 `make` 指令编译
```
[root@VM-8-2-centos lds_test]# make
gcc -c main.c -o main.o
gcc -c node2.c -o node2.o
gcc -c node1.c -o node1.o
gcc -lc main.o node2.o node1.o -o main -T main.lds
```
我们首先可以通过指令 `readelf -S node2.o` 查看 `node2.o` 包含的符号
```
[root@VM-8-2-centos lds_test]# readelf -S node2.o
There are 10 section headers, starting at offset 0x1b0:

Section Headers:
  [Nr] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
  [ 0]                   NULL             0000000000000000  00000000
       0000000000000000  0000000000000000           0     0     0
  [ 1] .text             PROGBITS         0000000000000000  00000040
       0000000000000000  0000000000000000  AX       0     0     1
  [ 2] .data             PROGBITS         0000000000000000  00000040
       0000000000000000  0000000000000000  WA       0     0     1
  [ 3] .bss              NOBITS           0000000000000000  00000040
       0000000000000000  0000000000000000  WA       0     0     1
  [ 4] __node2           PROGBITS         0000000000000000  00000040
       0000000000000004  0000000000000000   A       0     0     4
  [ 5] .comment          PROGBITS         0000000000000000  00000044
       000000000000002d  0000000000000001  MS       0     0     1
  [ 6] .note.GNU-stack   PROGBITS         0000000000000000  00000071
       0000000000000000  0000000000000000           0     0     1
  [ 7] .symtab           SYMTAB           0000000000000000  00000078
       00000000000000d8  0000000000000018           8     8     8
  [ 8] .strtab           STRTAB           0000000000000000  00000150
       000000000000000f  0000000000000000           0     0     1
  [ 9] .shstrtab         STRTAB           0000000000000000  0000015f
       000000000000004d  0000000000000000           0     0     1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  l (large), p (processor specific)
```
可以很明显看见 `Section Headers:` 内包含符号 `__node2`(`input section`)。
再次通过指令 `readelf -S main` 查看 `main` 包含的符号
```
[root@VM-8-2-centos lds_test]# readelf -S main | grep node
[root@VM-8-2-centos lds_test]#
```
并没有 `__node1, __node2`,但是通过 `objdump main -t | grep .rodata` 指令查询
```
[root@VM-8-2-centos lds_test]# objdump main -t | grep .rodata
0000000000400650 l    d  .rodata        0000000000000000              .rodata
000000000040066c g     O .rodata        0000000000000004              node1
0000000000400658 g     O .rodata        0000000000000000              .hidden __dso_handle
0000000000400650 g     O .rodata        0000000000000004              _IO_stdin_used
0000000000400670 g     O .rodata        0000000000000004              node2
```
会发现找到了符号 `node1 node2`在名为`.rodata`的`output section`内(和我们定义的`input/output section`关系是一致的),并且地址值相差为 4 。
```
[root@VM-8-2-centos lds_test]# diff s.lds main.lds
72,73c72,73
<   PROVIDE (etext = .);
<   .rodata         : { *(.rodata .rodata.* .gnu.linkonce.r.*) }
---
>   PROVIDE (etext = .);
>   .rodata         : { *(.rodata .rodata.*)  . = ALIGN(32); *(__node1)  *(__node2) *(.gnu.linkonce.r.*) }
```
修改 `main.lds` (与 s.lds 差异如上所示), 也就是在原有基础上添加 `. = ALIGN(32);`。
```
[root@VM-8-2-centos lds_test]# objdump main -t | grep .rodata
0000000000400650 l    d  .rodata        0000000000000000              .rodata
0000000000400680 g     O .rodata        0000000000000004              node1
0000000000400658 g     O .rodata        0000000000000000              .hidden __dso_handle
0000000000400650 g     O .rodata        0000000000000004              _IO_stdin_used
0000000000400684 g     O .rodata        0000000000000004              node2
```
可以看到 `node1` 的地址值从 `000000000040066c` 变为 `0000000000400680`, 这便是 `. = ALIGN(32);` 对齐的结果。但是 `node2` 和 `node1` 的地址差值仍然为 4 。
```
[root@VM-8-2-centos lds_test]# diff s.lds main.lds
72,73c72,73
<   PROVIDE (etext = .);
<   .rodata         : { *(.rodata .rodata.* .gnu.linkonce.r.*) }
---
>   PROVIDE (etext = .);
>   .rodata         : { *(.rodata .rodata.*) . = ALIGN(32); *(__node1) . = ALIGN(32); *(__node2) *(.gnu.linkonce.r.*) }
```
再在 `*(__node1)` 后添加一个 `. = ALIGN(32);`。
```
[root@VM-8-2-centos lds_test]# objdump main -t | grep .rodata
0000000000400650 l    d  .rodata        0000000000000000              .rodata
0000000000400680 g     O .rodata        0000000000000004              node1
0000000000400658 g     O .rodata        0000000000000000              .hidden __dso_handle
0000000000400650 g     O .rodata        0000000000000004              _IO_stdin_used
00000000004006a0 g     O .rodata        0000000000000004              node2
```
可以看到 `node2` 和 `node1` 的地址差值变为了 32。显然在 `section` 前添加 `. = ALIGN(32);` 仅可以将后**一个 `section`** 对齐(这关系到该 `patch` 组 `[RFC,3/4]` 的一个 `bug`, 后续会提及)。