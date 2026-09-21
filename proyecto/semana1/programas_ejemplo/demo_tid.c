//gcc -Wall -Wextra demo_tid.c -o demo_tid -pthread
#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static void *worker(void *arg)
{
    (void)arg;
    printf("hilo:  PID=%d TID=%d\n", (int)getpid(), (int)gettid());
    return NULL;
}

int main(void)
{
    pthread_t t;

    printf("main:  PID=%d TID=%d\n", (int)getpid(), (int)gettid());
    pthread_create(&t, NULL, worker, NULL);
    pthread_join(t, NULL);
    return 0;
}
