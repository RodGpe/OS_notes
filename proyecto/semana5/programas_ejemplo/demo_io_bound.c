/*
 * demo_io_bound.c — proceso I/O-bound: bloqueado en sleep (estado S en ps)
 *
 * Compilar: gcc -Wall -Wextra demo_io_bound.c -o demo_io_bound
 * Ejecutar: ./demo_io_bound [segundos_por_iteracion] [iteraciones]
 */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int sleep_sec = 2;
    int iters = 5;

    if (argc > 1)
        sleep_sec = atoi(argv[1]);
    if (argc > 2)
        iters = atoi(argv[2]);
    if (sleep_sec < 1)
        sleep_sec = 1;
    if (iters < 1)
        iters = 1;

    fprintf(stderr, "[io] pid=%d — %d x sleep(%d); observa stat=S en ps\n",
            (int)getpid(), iters, sleep_sec);

    for (int i = 0; i < iters; i++) {
        fprintf(stderr, "[io] iter %d — bloqueando...\n", i + 1);
        sleep((unsigned)sleep_sec);
    }

    fprintf(stderr, "[io] fin\n");
    return 0;
}
