#include <stdio.h>
#include <stdbool.h>
#include <sys/ptrace.h>
#include <signal.h>

void prog_timeout(int a1)
{
  int v1;
  v1 = sys_exit(1);
}

unsigned int enable_timeout_cons()
{
  signal(14, (__sighandler_t)prog_timeout);
  return alarm(0x3Cu);
}

bool auth(char *s, int serial)
{
  int i;
  int value;
  int len;

  s[strcspn(s, "\n")] = 0;
  len = strnlen(s, 32);
  if ( len <= 5 )
    return 1;
  if ( ptrace(PTRACE_TRACEME, 0, 1, 0) == -1 )
  {
    puts("\x1B[32m.---------------------------.");
    puts("\x1B[31m| !! TAMPERING DETECTED !!  |");
    puts("\x1B[32m'---------------------------'");
    return 1;
  }
  else
  {
    value = (s[3] ^ 4919) + 6221293;
    for ( i = 0; i < len; ++i )
    {
      if ( s[i] <= 31 )
        return 1;
      value += (value ^ (unsigned int)s[i]) % 1337;
    }
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
  scanf(&unk_8048A60, &serial);
  if ( auth(s, serial) )
    return 1;
  puts("Authenticated!");
  system("/bin/sh");
  return 0;
}