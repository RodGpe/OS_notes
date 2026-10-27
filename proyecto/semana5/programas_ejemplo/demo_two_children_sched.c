/*
 * demo_two_children_sched.c — padre lanza CPU-bound e I/O-bound a la vez
 *
 * Compilar: gcc -Wall -Wextra -O0 demo_two_children_sched.c -o demo_two_children_sched
 * Ejecutar: ./demo_two_children_sched
 *
 * Mientras corren: watch -n0.5 'ps -o pid,stat,pcpu,comm --ppid '"$(pgrep -n demo_two_children_sched)"
 */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static void child_cpu(int sec)
{
    volatile unsigned long acc = 0;
    struct timespec start, now;

    fprintf(stderr, "[cpu child] pid=%d\n", (int)getpid());
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (;;) {
        for (int i = 0; i < 800000; i++)
            acc += (unsigned long)i;
        clock_gettime(CLOCK_MONOTONIC, &now);
        if (now.tv_sec - start.tv_sec >= sec)
            break;
    }
    (void)acc;
    _exit(0);
}

static void child_io(int sec)
{
    fprintf(stderr, "[io child] pid=%d\n", (int)getpid());
    sleep((unsigned)sec);
    _exit(0);
}

int main(void)
{
    int dur = 6;
    pid_t a, b;

    fprintf(stderr, "[padre] pid=%d — fork CPU + IO (%d s)\n", (int)getpid(), dur);

    a = fork();
    if (a < 0) {
        perror("fork");
        return 1;
    }
    if (a == 0)
        child_cpu(dur);

    b = fork();
    if (b < 0) {
        perror("fork");
        return 1;
    }
    if (b == 0)
        child_io(dur);

    waitpid(a, NULL, 0);
    waitpid(b, NULL, 0);
    fprintf(stderr, "[padre] ambos hijos terminaron\n");
    return 0;
}
