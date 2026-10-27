/*
 * demo_sigaction_flag.c — handler seguro con volatile sig_atomic_t
 *
 * Patrón típico en servidores: el handler solo marca un flag; main hace el trabajo real.
 *
 * Compilar: gcc -Wall -Wextra demo_sigaction_flag.c -o demo_sigaction_flag
 * Ejecutar: ./demo_sigaction_flag
 *           Ctrl+C una o más veces; el proceso sale ordenadamente tras el loop.
 */
#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static volatile sig_atomic_t g_stop = 0;

static void on_sigint(int signo)
{
    (void)signo;
    g_stop = 1;
}

int main(void)
{
    struct sigaction sa;

    sa.sa_handler = on_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror("sigaction");
        return 1;
    }

    fprintf(stderr, "PID=%d — Ctrl+C marca g_stop; salida ordenada\n", (int)getpid());

    while (!g_stop) {
        fprintf(stderr, ".");
        sleep(1);
    }

    fprintf(stderr, "\n[g_stop=1] limpieza simulada y exit(0)\n");
    return 0;
}
