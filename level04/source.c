#include <sys/ptrace.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <signal.h>

int32_t clear_stdin()
{
    char var_d = 0;
    int32_t eax_1;
    while (true)
    {
        eax_1 = getchar();
        char var_d_1 = eax_1;
        if (var_d_1 == 0xa)
        {
            break;
        }
        if (var_d_1 == 0xff)
        {
            break;
        }
        if ((!(var_d_1 != 0xa && var_d_1 != 0xff)))
        {
            /* nop */
        }
    }
    return eax_1;
}

int32_t get_unum()
{
    int32_t var_10 = 0;
    fflush(stdout);
    scanf("%u", &var_10);
    clear_stdin();
    return var_10;
}

int32_t prog_timeout()
{
    int32_t ebp;
    int32_t var_4 = ebp;
    exit(1);
}

int32_t enable_timeout_cons()
{
    signal(SIGALRM, prog_timeout);
    return alarm(60);
}


int main(int argc, char** argv, char** envp)
{
    pid_t pid = fork();
    char s[128];
    memset(&s, 0, 128);
    int var_18 = 0;
    int stat_loc = 0;
    if (pid == 0)
    {
        prctl(PR_SET_PDEATHSIG, SIGHUP);
        ptrace(PTRACE_TRACEME, 0, 0, 0);
        puts("Give me some shellcode, k");
        gets(&s);
    }
    else
    {
        while (true)
        {
            wait(&stat_loc);
            if (((stat_loc & 0x7f) != 0 && (((stat_loc & 0x7f) + 1) >> 1) <= 0))
            {
                if (ptrace(PTRACE_PEEKUSER, eax, 44, 0) == 11)
                {
                    puts("no exec() for you");
                    kill(eax, 9);
                    break;
                }
                continue;
            }
            puts("child is exiting...");
            break;
        }
    }
    return 0;
}