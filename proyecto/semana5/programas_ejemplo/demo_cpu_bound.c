/*
 * demo_cpu_bound.c — proceso CPU-bound: poco I/O, mucho tiempo en RUNNING
 *
 * Compilar: gcc -Wall -Wextra -O0 demo_cpu_bound.c -o demo_cpu_bound
 * Ejecutar: ./demo_cpu_bound [segundos]
 *
 * En otra terminal: top -p $(pgrep -n demo_cpu_bound)
 * o: ps -o pid,stat,pcpu,comm -p $(pgrep -n demo_cpu_bound)
 */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static volatile unsigned long g_acc;

int main(int argc, char **argv)
{
    int secs = 5;
    struct timespec start, now;

    if (argc > 1)
        secs = atoi(argv[1]);
    if (secs < 1)
        secs = 1;

    fprintf(stderr, "[cpu] pid=%d — bucle caliente ~%d s (stat suele ser R)\n",
            (int)getpid(), secs);

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (;;) {
        for (int i = 0; i < 1000000; i++)
            g_acc += (unsigned long)i;
        clock_gettime(CLOCK_MONOTONIC, &now);
        if (now.tv_sec - start.tv_sec >= secs)
            break;
    }

    fprintf(stderr, "[cpu] fin acc=%lu\n", g_acc);
    return 0;
}
