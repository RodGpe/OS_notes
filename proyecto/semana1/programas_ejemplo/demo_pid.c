//gcc -Wall -Wextra demo_pid.c -o demo_pid
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    printf("MiniServ demo: PID=%d\n", (int)getpid());
    return 0;
}