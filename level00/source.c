#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int x;
    
    puts("***********************************");
    puts("* \t     -Level00 -\t\t  *");
    puts("***********************************");
    printf("Password:");
    scanf("%d", &x);
    if (x != 5276) {
        puts("\nInvalid Password!");
    }
    else {
        puts("\nAuthenticated!");
        system("/bin/sh");
    }
    return x != 5276;
}