/*
 * demo_ctxt_switches.c — lee voluntary/nonvoluntary context switches en /proc/self/status
 *
 * Compilar: gcc -Wall -Wextra -O0 demo_ctxt_switches.c -o demo_ctxt_switches
 * Ejecutar: ./demo_ctxt_switches
 */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static void read_ctxt_fields(const char *label)
{
    FILE *f = fopen("/proc/self/status", "r");
    char line[256];
    long vol = -1, nonvol = -1;

    if (!f) {
        perror("fopen /proc/self/status");
        return;
    }
    while (fgets(line, sizeof line, f)) {
        if (strncmp(line, "voluntary_ctxt_switches:", 24) == 0)
            vol = strtol(line + 24, NULL, 10);
        else if (strncmp(line, "nonvoluntary_ctxt_switches:", 27) == 0)
            nonvol = strtol(line + 27, NULL, 10);
    }
    fclose(f);
    fprintf(stderr, "[%s] voluntary=%ld nonvoluntary=%ld\n", label, vol, nonvol);
}

static void cpu_work(int seconds)
{
    volatile unsigned long acc = 0;
    struct timespec start, now;

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (;;) {
        for (int i = 0; i < 500000; i++)
            acc += (unsigned long)i;
        clock_gettime(CLOCK_MONOTONIC, &now);
        if (now.tv_sec - start.tv_sec >= seconds)
            break;
    }
    (void)acc;
}

int main(void)
{
    fprintf(stderr, "[demo] pid=%d — compara ctxt switches antes/después de CPU y sleep\n",
            (int)getpid());

    read_ctxt_fields("inicio");
    cpu_work(2);
    read_ctxt_fields("tras CPU ~2s");
    sleep(2);
    read_ctxt_fields("tras sleep 2s");

    return 0;
}
