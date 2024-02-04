#include <stdio.h>
#include <stdbool.h>
#include <string.h>

int get_unum()
{
    unsigned int v0;
    void* v1;

    v1 = 0;
    fflush(stdout);
    v0 = &v1;
    scanf("%u");
    clear_stdin();
    return v1;
}


int store_number(int buffer)
{
  int number;
  int index;
  int ret;
  
  printf(" Number: ");
  number = get_unum();
  printf(" Index: ");
  index = get_unum();
  if ((index % 3 == 0) || (number >> 24 == 183)) {
    puts(" *** ERROR! ***");
    puts("   This index is reserved for wil!");
    puts(" *** ERROR! ***");
    ret = 1;
  }
  else {
    // assign the value of number at the index of the buffer
    *(unsigned int *)(index * 4 + buffer) = number;
    ret = 0;
  }
  return ret;
}

int read_number(int arg1)
{
    int var_10 = 0;
    printf(" Index: ");
    int nb = get_unum();
    printf(" Number at data[%u] is %u\n", nb, *((nb << 2) + arg1));
    return 0;
}

int main(int argc, char** argv, char** envp)
{
    char** args = argv;
    char** env = envp;
    void* gsbase;
    int eax_2 = *(gsbase + 20);
    int var_2c;
    memset(var_2c, 0, 24);
    int buffer[100];
    memset(&buffer, 0, 400);
    int* var_1dc;
    size_t var_1d8;
    // clear the args
    while (*args != 0)
    {
        int ecx_1 = 0xffffffff;
        char* edi_1 = *args;
        while (ecx_1 != 0)
        {
            bool cond:0_1 = 0 != *edi_1;
            edi_1 = &edi_1[1];
            ecx_1 = (ecx_1 - 1);
            if ((!cond:0_1))
            {
                break;
            }
        }
        var_1d8 = ((!ecx_1) - 1);
        var_1dc = nullptr;
        memset(*args, 0, var_1d8);
        args = &args[1];
    }
    // clear the env
    while (*env != 0)
    {
        int ecx_2 = 0xffffffff;
        char* edi_2 = *env;
        while (ecx_2 != 0)
        {
            bool cond:1_1 = 0 != *edi_2;
            edi_2 = &edi_2[1];
            ecx_2 = (ecx_2 - 1);
            if ((!cond:1_1))
            {
                break;
            }
        }
        var_1d8 = ((!ecx_2) - 1);
        var_1dc = nullptr;
        memset(*env, 0, var_1d8);
        env = &env[1];
    }
    puts(
      "----------------------------------------------------\n  Welcome to wil\'s crappy number storage service!   \n----------------------------------------------------\n Commands:                                          \n    store - store a number into the data storage    \n    read  - read a number from the data storage     \n    quit  - exit the program                        \n----------------------------------------------------\n   wil has reserved some storage :>                 \n----------------------------------------------------\n"
      );
    while (true)
    {
        printf("Input command: ");
        int result = 1;
        char input[20] = {0};
        fgets(&input, 20, stdin);
        input[strcspn(&input, "\n")] = 0;

        if (!strncmp(&input, "store", 5))
            result = store_number(&buffer);
        if (!strncmp(&input, "read", 4))
            result = read_number(&buffer);
        if (!strncmp(&input, "quit", 4))
            return 0;
        if (result == 0)
            printf(" Completed %s command successful…", input);
        else
            printf(" Failed to do %s command\n", input);
        memset(input, 0, 20);
    }
}