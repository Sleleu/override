Compared to previous levels, we can observe that new protections have been added:

```bash
RELRO           STACK CANARY      NX            PIE             RPATH      RUNPATH      FILE
Partial RELRO   Canary found      NX enabled    No PIE          No RPATH   No RUNPATH   /home/users/level03/level03
```

We can see that stack canary protection has been activated, as well as NX. For some details on these protections:

- https://beta.hackndo.com/canary-bypass-technique/
- https://www.bases-hacking.org/aslr-nx.html

Here's the flag:
```bash
level03@OverRide:~$ ./level03 
***********************************
*               level03           **
***********************************
Password:322424827
$ ls
ls: cannot open directory .: Permission denied
$ whoami
level04
$ cat /home/users/level04/.pass  
kgv3tkEb9h2mLkRsPkXRfc2mHbjMxQzvb2FrgKkf
```

**Why does it work?!**

Because 322424845 - 322424827 = 18.

Let's go back to the program !

```c
int main(int argc, const char **argv, const char **envp)
{
  unsigned int v3;
  int savedregs;

  v3 = time(0);
  srand(v3);
  puts("***********************************");
  puts("*\t\tlevel03\t\t**");
  puts("***********************************");
  printf("Password:");
  scanf("%d", &savedregs);
  test(savedregs, 322424845);
  return 0;
}
```

The savedregs variable will retrieve an int from our input. 
This int will be sent, along with the value `322424845` to a `test()` function:

```c
int test(int a1, int a2)
{
  int result;
  char random;

  switch ( a2 - a1 )
  {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
      result = decrypt(a2 - a1);
      break;
    default:
      random = rand();
      result = decrypt(random);
      break;
  }
  return result;
}
```

We see that in cases where the result of 322424845 - input = 1 to 21, the result is sent to the `decrypt()` function, 
which will use this result as a XOR encryption key. Otherwise, it's a totally random value.

```c
int decrypt(char a1)
{
  unsigned int i;
  unsigned int len;
  char buffer[29];

  strcpy(buffer, "Q}|u`sfg~sf{}|a3");
  len = strlen(buffer);
  for ( i = 0; i < len; ++i )
    buffer[i] ^= a1;
  if ( !strcmp(buffer, "Congratulations!") )
    return system("/bin/sh");
  else
    return puts("\nInvalid Password");
}
```

XOR is an operation performed on the bits of 2 values. If the comparison shows that the two bits are identical, the result will be false | 0, and if they are different, XOR returns true | 1:
```
01000001   (65 / ascii A)
XOR
01000010   (66 / ascii B)
--------
00000011   (3)
```
The cool thing is that **XOR is reversible**, for example, a XOR between 3 and 66 will yield 65, and a XOR between 3 and 65 yield 66.


In this function, we see that the string **Q}|u`sfg~sf{}|a3** must equal **Congratulations!** after applying the XOR. We can simply test the possible results from 1 to 21 (since there aren't many) to get the flag.
So, to confirm this, we can perform a XOR decryption in practice to illustrate how the program works.

As an example, we can use my [xorcipher script](https://github.com/Sleleu/xorcipher) (🙂) to apply a XOR of 18 on each character of the string **Q}|u`sfg~sf{}|a3**

The usage is very simple.

For encryption:
```bash
python3 xorcipher.py -e <key> <message>
```

For decryption:
```bash
python3 xorcipher.py -d <key> <encrypted>
```

Just encrypt the text with a single key of value `0`, because `x ^ 0 = x`:
```bash
python3 xorcipher.py -e '0' 'Q}|u`sfg~sf{}|a3'
81 125 124 117 96 115 102 103 126 115 102 123 125 124 97 51
```

And finally use the key `18` to obtain the string:
```bash
python3 xorcipher.py -d '18' '81 125 124 117 96 115 102 103 126 115 102 123 125 124 97 51'
Congratulations!
```
