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