To begin with, let's take a look at the decompiled code.

To bypass this authentication part, we need a login of at least 6 characters:
```c
s[strcspn(s, "\n")] = 0;
v5 = strnlen(s, 32);
if ( v5 <= 5 )
  return 1;
```

This part of the code is meant to prevent the use of a debugger. 
We can't rely on it at this level (see, for example: https://www.bases-hacking.org/ptrace.html):
```c
if ( ptrace(PTRACE_TRACEME, 0, 1, 0) == -1 )
{
  puts("\x1B[32m.---------------------------.");
  puts("\x1B[31m| !! TAMPERING DETECTED !!  |");
  puts("\x1B[32m'---------------------------'");
  return 1;
}
```
There are XOR operations scattered throughout this part of the code, and the goal is to obtain the same value
between the serial and value variables so that the auth() function returns 0 instead of 1, which should then lead to system("/bin/sh"):

```c
value = (s[3] ^ 4919) + 6221293;
for ( i = 0; i < len; ++i )
{
  if ( s[i] <= 31 )
  {
    printf("s[i]: %d\n", s[i]);
    return 1;
  }
  value += (value ^ (unsigned int)s[i]) % 1337;
}
printf("Serial: %d\n", serial);
printf("Value: %d\n", value);
return serial != value;

int main(int argc, const char **argv, const char **envp)
{
 ...
if ( auth(s, serial) )
  return 1;
puts("Authenticated!");
system("/bin/sh");
return 0;
}
```

Here's the flag!
```bash
level06@OverRide:~$ ./level06 
***********************************
*		level06		  *
***********************************
-> Enter Login: 6232627
***********************************
***** NEW ACCOUNT DETECTED ********
***********************************
-> Enter Serial: 6232627
Authenticated!
$ whoami
level07
$ cat /home/users/level07/.pass
GbcPDRgsFK77LNnnuh7QyFYA2942Gp8yKj9KrWD8
```


I simply recompiled my own version of the source code, adding printf statements to see the processed values:
```c
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

bool auth(char *s, int serial)
{
  int i;
  int value;
  int len;

  s[strcspn(s, "\n")] = 0;
  len = strnlen(s, 32);
  if ( len <= 5 ) {
    printf("len: %d\n", len);
    return 1;
  }
  else
  {
    value = (s[3] ^ 4919) + 6221293;
    for ( i = 0; i < len; ++i )
    {
      if ( s[i] <= 31 )
      {
        printf("s[i]: %d\n", s[i]);
        return 1;
      }
      value += (value ^ (unsigned int)s[i]) % 1337;
    }
    printf("Serial: %d\n", serial);
    printf("Value: %d\n", value);
    return serial != value;
  }
}

int main(int argc, const char **argv, const char **envp)
{
  int serial;
  char s[28];

  puts("***********************************");
  puts("*\t\tlevel06\t\t  *");
  puts("***********************************");
  printf("-> Enter Login: ");
  fgets(s, 32, stdin);
  puts("***********************************");
  puts("***** NEW ACCOUNT DETECTED ********");
  puts("***********************************");
  printf("-> Enter Serial: ");
  scanf("%u", &serial);
  if ( auth(s, serial) )
    return 1;
  puts("Authenticated!");
  system("/bin/sh");
  return 0;
}
```

and played with those values:
```bash
➜  level06 git:(main) ✗ ./a.out
***********************************
*               level06           *
***********************************
-> Enter Login: 123456
***********************************
***** NEW ACCOUNT DETECTED ********
***********************************
-> Enter Serial: 123456
Serial: 123456
Value: 6231517
➜  level06 git:(main) ✗ ./a.out
***********************************
*               level06           *
***********************************
-> Enter Login: 123456
***********************************
***** NEW ACCOUNT DETECTED ********
***********************************
-> Enter Serial: 123456
Serial: 123456
Value: 6231517
➜  level06 git:(main) ✗ ./a.out
***********************************
*               level06           *
***********************************
-> Enter Login: 123456
***********************************
***** NEW ACCOUNT DETECTED ********
***********************************
-> Enter Serial: 1234567
Serial: 1234567
Value: 6231517
➜  level06 git:(main) ✗ ./a.out
***********************************
*               level06           *
***********************************
-> Enter Login: 6234517
***********************************
***** NEW ACCOUNT DETECTED ********
***********************************
-> Enter Serial: 

1234567
Serial: 1234567
Value: 6232627
➜  level06 git:(main) ✗ ./a.out
***********************************
*               level06           *
***********************************
-> Enter Login: 6232627
***********************************
***** NEW ACCOUNT DETECTED ********
***********************************
-> Enter Serial: 6232627
Serial: 6232627
Value: 6232627
Authenticated!
```
Then, I just needed to enter the same values into the VM executable 🙂

BUT, let's try to understand a bit of the code and why it worked. We should focus on this part:
```c
value = (s[3] ^ 4919) + 6221293;
    for ( i = 0; i < len; ++i )
    {
      if ( s[i] <= 31 )
        return 1;
      value += (value ^ (unsigned int)s[i]) % 1337;
    }
    return serial != value;
```

- `value = (s[3] ^ 4919) + 6221293`: Initializes value with a XOR between the 4th character of the login and 4919, then adds 6221293 to it.
- `if ( s[i] <= 31 ) return 1;`: Excludes non-printable characters from this login.
- `value += (value ^ (unsigned int)s[i]) % 1337;`: Adds to `value` the result of a XOR with each character of the login, modulo 1337.

We will try to break down the program's logic step by step, starting with the login `123456`.

For the initialization of `value`, starting from the login `123456`:

4 is the 4th character, so 4 = 52 in decimal.

- 52 ^ 4919 = 4867.
- 4867 + 6221293 = **6226160**.
- value will be equal to **6226160**.

Then, for each character, `value` adds its own value XOR `(unsigned int)s[i] % 1337`.

So, for the first character, which is 1, ASCII 49:

`value += (6226160 ^ 49) % 1337` corresponds to:

- 6226160 ^ 49 = 6226113.
- 6226113 % 1337 = **1041**.

So, value += 1041, value = **6227201**.

And so on...

| Character | Operation         | Result   | Final Value   |
|-----------|-------------------|----------|---------------|
| 2e        | 6227201 ^ 50      | 6227251  |               |
|           | 6227251 % 1337    | 842      | 6228043       |
| 3e        | 6228043 ^ 51      | 6228088  |               |
|           | 6228088 % 1337    | 342      | 6228385       |
| 4e        | 6228385 ^ 52      | 6228373  |               |
|           | 6228373 % 1337    | 627      | 6229012       |
| 5e        | 6229012 ^ 53      | 6229025  |               |
|           | 6229025 % 1337    | 1279     | 6230291       |
| 6e        | 6230291 ^ 54      | 6230309  |               |
|           | 6230309 % 1337    | 1226     | 6231517       |

Exactly this result for `123456`:
```bash
➜  level06 git:(main) ✗ ./a.out
***********************************
*               level06           *
***********************************
-> Enter Login: 123456
***********************************
***** NEW ACCOUNT DETECTED ********
***********************************
-> Enter Serial: 123456
Serial: 123456
Value: 6231517
```

We can confirm this execution by adding some printf to our recompiled code:

```
***********************************
*               level06           *
***********************************
-> Enter Login: 123456
***********************************
***** NEW ACCOUNT DETECTED ********
***********************************
-> Enter Serial: 123456
initial value: 6226160
value[0]: 6227201
value[1]: 6228043
value[2]: 6228385
value[3]: 6229012
value[4]: 6230291
value[5]: 6231517
Serial: 123456
Value: 6231517
```

The trick, then, is simply to enter a username once to retrieve the serial as a result, 
in order to use the obtained serial for the next authentication with the same username. 
It is equally possible to obtain it directly from gdb as well.

I think we've got it all figured out!
