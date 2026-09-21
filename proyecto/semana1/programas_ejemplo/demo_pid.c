//gcc -Wall -Wextra demo_pid.c -o demo_pid
#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    printf("MiniServ demo: PID=%d TID=%d\n", (int)getpid(), (int)gettid());
    return 0;
}