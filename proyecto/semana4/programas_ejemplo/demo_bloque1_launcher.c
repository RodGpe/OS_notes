/*
 * demo_bloque1_launcher.c — ejecuta miniserv con SIGINT en acción por defecto
 *
 * Los procesos arrancados con "cmd &" en bash suelen tener SIGINT ignorado (job control).
 * Este launcher restaura SIG_DFL antes de execv, para que kill -INT se comporte como Ctrl+C.
 *
 * Compilar:
 *   gcc -Wall -Wextra demo_bloque1_launcher.c -o demo_bloque1_launcher
 *
 * Uso (desde el script de demo):
 *   ./demo_bloque1_launcher /ruta/a/miniserv
 */
#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    struct sigaction sa;

    if (argc < 2) {
        fprintf(stderr, "Uso: %s /ruta/miniserv\n", argv[0]);
        return 1;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror("sigaction SIGINT");
        return 1;
    }
    if (sigaction(SIGTERM, &sa, NULL) < 0) {
        perror("sigaction SIGTERM");
        return 1;
    }

    execv(argv[1], &argv[1]);
    perror("execv");
    return 1;
}
