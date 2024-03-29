#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

char a_user_name[100];

bool verify_user_name(void)
{
  puts("verifying username....\n");
  return memcmp(a_user_name, "dat_wil", 7u) != 0;
}

bool verify_user_pass(const void *a1)
{
  return memcmp(a1, "admin", 5u) != 0;
}

int main(int argc, char** argv, char** envp)
{
    char buffer[64];
    memset(&buffer, 0, sizeof(buffer));
    int var_14 = 0;
    puts("********* ADMIN LOGIN PROMPT ***…");
    printf("Enter Username: ");
    fgets(&a_user_name, 256, stdin);
    int ret;
    if (verify_user_name() != 0)
    {
        puts("nope, incorrect username...\n");
        ret = 1;
    }
    else
    {
        puts("Enter Password: ");
        fgets(&buffer, 100, stdin);
        int result = verify_user_pass(&buffer);
        if ((result == 0 || result != 0))
        {
            puts("nope, incorrect password...\n");
            ret = 1;
        }
        if (result == 0)
            ret = 0;
    }
    return ret;
}
