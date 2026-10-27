/*
 * demo_catch_signal_simple.c — capturar (casi) todas las señales e imprimir número
 *
 * Propósito didáctico (Semana 5):
 *   - Ver el mecanismo básico: sigaction + handler void f(int signo).
 *   - Experimentar con muchas señales sin el ruido de siginfo_t ni async-signal-safe.
 *   - Contrastar con demo_catch_all_signals.c (write + g_stop) y
 *     demo_catch_signal_siginfo.c (SA_SIGINFO).
 *
 * Instala handler en señales 1..NSIG-1 excepto SIGKILL y SIGSTOP.
 *
 * IMPORTANTE (estilo de código):
 *   Aquí usamos printf dentro del handler solo para clase. En MiniServ V4 y en
 *   demo_catch_all_signals.c el handler no debe usar stdio: ver signal-safety(7).
 *
 * Compilar:
 *   gcc -Wall -Wextra demo_catch_signal_simple.c -o demo_catch_signal_simple
 *
 * Ejecutar:
 *   ./demo_catch_signal_simple
 *
 * Probar:
 *   Ctrl+C              → SIGINT (2), mensaje, proceso sigue
 *   Ctrl+Z              → SIGTSTP (20), mensaje; no confundir con SIGSTOP
 *   kill -TERM <pid>    → capturada
 *   kill -HUP  <pid>    → capturada
 *   kill -STOP <pid>    → SIGSTOP: NO capturable, queda en T (detenido)
 *   kill -CONT <pid>    → tras STOP
 *   kill -9    <pid>    → SIGKILL: no capturable, muere
 *
 * strace:
 *   strace -e trace=signal ./demo_catch_signal_simple
 */
#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#ifndef NSIG
#define NSIG 32
#endif

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/*
 * Handler clásico de una sola argumento (sin SA_SIGINFO).
 * El kernel solo pasa el número de señal; no hay siginfo_t ni contexto de CPU.
 *
 * Tras ejecutar el handler, el proceso continúa donde se interrumpió (salvo
 * señales cuya acción por defecto era Terminar y nosotros la reemplazamos).
 */
static void al_recibir(int signo)
{
    /*
     * Didáctico: printf + fflush. En producción → write(2) o solo un flag
     * volatile sig_atomic_t leído en main (patrón MiniServ).
     */
    printf("\nRecibí la señal %d\n", signo);
    fflush(stdout);
}

static int instalar_todas(void)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = al_recibir;
    sigemptyset(&sa.sa_mask);
    /*
     * sa_mask: señales a bloquear mientras corre el handler (evita reentrada).
     * Vacío = no bloqueamos extra; el kernel ya bloquea la señal actual y
     * SIGKILL/SIGSTOP durante el handler.
     */
    sa.sa_flags = SA_RESTART;

    /*
     * Registrar el mismo handler en (casi) todas las señales estándar.
     *
     * Omitimos:
     *   SIGKILL (9)  — kill -9; el kernel no permite handler ni ignorar.
     *   SIGSTOP (19) — kill -STOP; detención forzada; sin handler user space.
     *
     * Ctrl+Z en terminal envía SIGTSTP (20), no SIGSTOP; esa sí la capturamos.
     *
     * sigaction() falla con EINVAL en algunos índices sin señal → ignoramos.
     */
    for (int sig = 1; sig < NSIG; sig++) {
        if (sig == SIGKILL || sig == SIGSTOP)
            continue;
        if (sigaction(sig, &sa, NULL) < 0 && errno != EINVAL) {
            perror("sigaction");
            return -1;
        }
    }
    return 0;
}

int main(void)
{
    if (instalar_todas() < 0)
        return 1;

    printf("PID=%d — handlers en señales 1..%d excepto SIGKILL/SIGSTOP\n",
           (int)getpid(), NSIG - 1);
    printf("Ctrl+C (SIGINT), Ctrl+Z (SIGTSTP), kill -TERM; kill -9 no se captura\n");

    /*
     * pause() bloquea hasta que llegue cualquier señal capturada.
     * Cada señal despierta pause(), corre al_recibir(), y volvemos a pause().
     * No hay salida ordenada: INT/TERM también dejan el proceso vivo (a diferencia
     * de demo_catch_all_signals.c que usa g_stop para exit limpio).
     *
     * Para apagar: kill -9 o cerrar terminal / matar desde fuera.
     */
    while (1)
        pause();

    return 0;
}
