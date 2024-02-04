Exactly the same thing as for the [first level of rainfall](https://github.com/Sleleu/Rainfall/tree/main/level0),
we just need to print the value at the instruction:

```nasm
0x080484e7 <+83>:	cmp    $0x149c,%eax
```

The password is simply written in clear in the program
```nasm
(gdb) p 0x149c
$6 = 5276
```

All that remains is to enter the password `5276`:
```
level00@OverRide:~$ ./level00 
***********************************
*        -Level00 -          *
***********************************
Password:5276

Authenticated!
$ ls
level00
$ whoami 
level01
```

And here is the flag:

```
$ cat /home/users/level01/.pass
uSq2ehEGT6c9S24zbshexZQBXUGrncxn5sD5QfGL
```
