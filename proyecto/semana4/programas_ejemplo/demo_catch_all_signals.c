/*
 * demo_catch_all_signals.c — capturar (casi) todas las señales: "difícil de matar"
 *
 * Propósito didáctico (Semana 5):
 *   - Contrastar señales capturables vs. SIGKILL/SIGSTOP (acción fija en el kernel).
 *   - Ver por qué systemd/docker hacen SIGTERM y luego, si hace falta, SIGKILL.
 *   - Practicar reglas de handlers (async-signal-safe) frente a demo_catch_signal_simple.c
 *     (que usa printf en el handler solo para enseñar, no como modelo de producción).
 *
 * Compilar:
 *   gcc -Wall -Wextra demo_catch_all_signals.c -o demo_catch_all_signals
 *
 * Ejecutar:
 *   ./demo_catch_all_signals
 *
 * Probar en otra terminal (sustituir PID):
 *   kill -TERM <pid>    # capturada; al cabo de un momento sale (g_stop)
 *   kill -INT  <pid>    # igual
 *   kill -HUP  <pid>    # capturada; el proceso sigue (no pone g_stop)
 *   kill -STOP <pid>    # SIGSTOP — NO capturable; queda detenido (T en ps)
 *   kill -CONT <pid>    # reanuda tras STOP
 *   Ctrl+Z              # SIGTSTP (20) — SÍ capturable aquí; ver mensaje en stderr
 *   kill -9    <pid>    # SIGKILL — no capturable; termina sí o sí
 *
 * strace:
 *   strace -e trace=signal ./demo_catch_all_signals
 *
 * Ver también:
 *   demo_catch_signal_simple.c   — mismo bucle de instalación, handler con printf
 *   demo_catch_signal_siginfo.c  — SA_SIGINFO y campos de siginfo_t
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
 * Flag compartido handler ↔ main. Debe ser sig_atomic_t y volatile:
 *   - volatile: el compilador no puede optimizar lecturas en el while (!g_stop)
 *   - sig_atomic_t: tipo POSIX garantizado para asignación atómica en handlers
 *
 * Patrón de servidores (MiniServ V4): el handler solo marca flags; el cierre
 * real (close listen_fd, wait hijos) ocurre en el loop principal, no en el handler.
 */
static volatile sig_atomic_t g_stop = 0;

/*
 * Convierte un entero a decimal usando solo write().
 * Los handlers de señal no deben usar printf/snprintf (no async-signal-safe):
 * si el main fue interrumpido dentro de malloc/stdio, printf en el handler
 * puede deadlock. Ver: man 7 signal-safety en Linux.
 */
static void write_u32(int fd, unsigned int v)
{
    char tmp[12];
    int i = 0;

    if (v == 0) {
        (void)write(fd, "0", 1);
        return;
    }
    while (v > 0) {
        tmp[i++] = (char)('0' + (v % 10));
        v /= 10;
    }
    while (i > 0)
        (void)write(fd, &tmp[--i], 1);
}

static void on_catchable(int signo)
{
    /*
     * Solo funciones async-signal-safe (write, _exit, waitpid en casos acotados…).
     * No: printf, malloc, free, exit, pthread_mutex_lock, etc.
     */
    (void)write(STDERR_FILENO, "\n[capturada] signo=", 18);
    write_u32(STDERR_FILENO, (unsigned int)signo);
    (void)write(STDERR_FILENO, "\n", 1);

    /*
     * Simulamos "apagado ordenado" solo para INT/TERM (como haría MiniServ).
     * Otras señales (SIGHUP, SIGUSR1, SIGTSTP…) solo se registran en stderr;
     * el proceso sigue en el loop salvo que uses kill -9 o SIGSTOP.
     */
    if (signo == SIGINT || signo == SIGTERM)
        g_stop = 1;
}

static int install_catch_all(void)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_catchable;
    sigemptyset(&sa.sa_mask);
    /*
     * SA_RESTART: si una señal llega durante sleep(), sleep puede reiniciarse
     * en lugar de fallar con EINTR. Útil en demos; en servidores a veces se
     * prefiere EINTR explícito para revisar flags de apagado tras accept().
     */
    sa.sa_flags = SA_RESTART;

    /*
     * Bucle 1 .. NSIG-1: instalar el mismo handler en cada número de señal.
     *
     * Excepciones obligatorias del kernel (no hay handler en user space):
     *
     *   SIGKILL (9)  — "kill -9"; acción fija Term; no se captura ni ignora.
     *   SIGSTOP (19) — "kill -STOP"; acción fija Stop; usada por depuradores/job control.
     *
     * Confusión frecuente en clase:
     *   Ctrl+Z no envía SIGSTOP, envía SIGTSTP (20), que SÍ podemos capturar aquí.
     *   Por eso con este demo Ctrl+Z imprime "[capturada] signo=20" y a menudo
     *   el proceso NO se suspende (reemplazamos la acción por defecto).
     *
     * Capturar SIGCONT/SIGTSTP en un servicio real suele ser mala idea; aquí es
     * solo para experimentar. MiniServ V4 no necesita capturarlas.
     */
    for (int sig = 1; sig < NSIG; sig++) {
        if (sig == SIGKILL || sig == SIGSTOP)
            continue;
        if (sigaction(sig, &sa, NULL) < 0) {
            /* En algunos números NSIG no hay señal real → EINVAL; ignorar. */
            if (errno != EINVAL) {
                perror("sigaction");
                return -1;
            }
        }
    }
    return 0;
}

int main(void)
{
    if (install_catch_all() < 0)
        return 1;

    dprintf(STDERR_FILENO,
            "PID=%d — handlers en 1..%d salvo SIGKILL/SIGSTOP\n"
            "  kill -TERM %d  → capturada; sale cuando g_stop (SIGINT/TERM)\n"
            "  kill -9 %d     → SIGKILL, no capturable\n"
            "  Ctrl+C (foreground) → SIGINT → g_stop\n"
            "  Ctrl+Z → SIGTSTP (capturable); kill -STOP usa SIGSTOP (no)\n",
            (int)getpid(), NSIG - 1, (int)getpid(), (int)getpid());

    /*
     * Loop cooperativo en lugar de pause() infinito:
     * permite salir ordenadamente cuando g_stop == 1 tras INT/TERM capturadas.
     * (Con pause() solo también funcionaría: cada señal despertaría pause, pero
     * habría que comprobar g_stop tras cada retorno.)
     */
    while (!g_stop) {
        dprintf(STDERR_FILENO, ".");
        sleep(1);
    }

    dprintf(STDERR_FILENO,
            "\n[g_stop=1] salida ordenada (tras SIGINT o SIGTERM capturada)\n");
    return 0;
}
