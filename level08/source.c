#include <stdio.h>
#include <stdbool.h>

unsigned int log_wrapper(FILE *a1, const char *a2, const char *a3)
{
  char dest[264];
  unsigned int v6;

  v6 = __readfsqword(40);
  strcpy(dest, a2);
  snprintf(&dest[strlen(dest)], 254 - strlen(dest), a3);
  dest[strcspn(dest, "\n")] = 0;
  fprintf(a1, "LOG: %s\n", dest);
  return __readfsqword(40) ^ v6;
}

int  main(int argc, const char **argv, const char **envp)
{
  FILE *log;
  FILE *stream;
  int fd;
  char buf = -1;
  char dest[104];

  if (argc != 2)
    printf("Usage: %s filename\n", *argv);
  log = fopen("./backups/.log", "w");
  if (!log)
  {
    printf("ERROR: Failed to open %s\n", "./backups/.log");
    exit(1);
  }
  log_wrapper(log, "Starting back up: ", argv[1]);
  stream = fopen(argv[1], "r");
  if (!stream)
  {
    printf("ERROR: Failed to open %s\n", argv[1]);
    exit(1);
  }
  strcpy(dest, "./backups/");
  strncat(dest, argv[1], 99 - strlen(dest));
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
  log_wrapper(log, "Finished back up ", argv[1]);
  fclose(stream);
  close(fd);
  return 0;
}