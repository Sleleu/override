#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int secret_backdoor()
{
  char s[128];

  fgets(s, 128, stdin);
  return system(s);
}

int handle_msg()
{
  char buffer[140];
  __int64_t v2;
  __int64_t v3;
  __int64_t v4;
  __int64_t v5;
  __int64_t v6;
  int v7 = 140;

  //memset of 40 bytes
  v2 = 0LL;
  v3 = 0LL;
  v4 = 0LL;
  v5 = 0LL;
  v6 = 0LL;

  set_username((__int64_t)buffer);
  set_msg((__int64_t)buffer);
  return puts(">: Msg sent!");
}

char *set_msg(__int64_t buffer)
{
  char s[1024];

  memset(s, 0, sizeof(s));
  puts(">: Msg @Unix-Dude");
  printf(">>: ");
  fgets(s, 1024, stdin);
  return strncpy((char *)buffer, s, *(int *)(buffer + 180));
}

int set_username(__int64_t buffer)
{
  char s[140];
  int i;

  memset(s, 0, 128uLL);
  puts(">: Enter your username");
  printf(">>: ");
  fgets(s, 128, stdin);
  for ( i = 0; i <= 40 && s[i]; ++i )
    *(_BYTE *)(buffer + i + 140) = s[i];
  return printf(">: Welcome, %s", (const char *)(buffer + 140));
}

int main(int argc, char** argv, char** envp)
{
    puts(
    "--------------------------------------------\n"
    "|   ~Welcome to l33t-m$n ~    v1337        |\n"
    "--------------------------------------------");
    handle_msg();
    return 0;
}