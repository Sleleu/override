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