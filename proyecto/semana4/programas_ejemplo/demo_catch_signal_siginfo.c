/*
 * demo_catch_signal_siginfo.c — capturar señal e imprimir siginfo_t
 *
 * Ver también: demo_catch_signal_simple.c (solo número de señal)
 *
 * Usa SA_SIGINFO para que el kernel pase metadatos en siginfo_t.
 * (Didáctico: printf en el handler; en servidores reales usar write/flags.)
 *
 * Compilar:
 *   gcc -Wall -Wextra demo_catch_signal_siginfo.c -o demo_catch_signal_siginfo
 *
 * Ejecutar:
 *   ./demo_catch_signal_siginfo
 *
 * Probar:
 *   Ctrl+C
 *   kill -TERM <pid>
 *   kill -HUP  <pid>
 *   kill -9 <pid>              # SIGKILL no se captura
 *   (fork + wait en otro demo para ver SIGCHLD con si_status)
 */
#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static const char *si_code_nombre(int signo, int code)
{
    if (signo == SIGCHLD) {
        switch (code) {
        case CLD_EXITED:    return "CLD_EXITED (hijo terminó con exit/_exit)";
        case CLD_KILLED:    return "CLD_KILLED (hijo murió por señal)";
        case CLD_DUMPED:    return "CLD_DUMPED (hijo murió + core)";
        case CLD_TRAPPED:   return "CLD_TRAPPED (trapped)";
        case CLD_STOPPED:   return "CLD_STOPPED (hijo detenido)";
        case CLD_CONTINUED: return "CLD_CONTINUED (hijo reanudado)";
        default:            return "(código CLD desconocido)";
        }
    }

    switch (code) {
    case SI_USER:     return "SI_USER (kill(), raise(), etc.)";
    case SI_KERNEL:   return "SI_KERNEL (kernel, p. ej. terminal)";
    case SI_QUEUE:    return "SI_QUEUE (sigqueue())";
    case SI_TIMER:    return "SI_TIMER (timer POSIX)";
    case SI_MESGQ:    return "SI_MESGQ (cola de mensajes)";
    case SI_ASYNCIO:  return "SI_ASYNCIO (aio)";
    case SI_SIGIO:    return "SI_SIGIO (SIGIO)";
    case SI_TKILL:    return "SI_TKILL (tkill/tgkill)";
    case SI_DETHREAD: return "SI_DETHREAD";
    default:          return "(otro si_code)";
    }
}

static void al_recibir(int signo, siginfo_t *info, void *ctx)
{
    (void)ctx;

    printf("\n--- señal capturada ---\n");
    printf("  signo (arg):     %d\n", signo);
    if (info == NULL) {
        printf("  (sin siginfo_t)\n");
        fflush(stdout);
        return;
    }

    printf("  si_signo:        %d\n", info->si_signo);
    printf("  si_errno:        %d", info->si_errno);
    if (info->si_errno != 0)
        printf(" (%s)", strerror(info->si_errno));
    printf("\n");
    printf("  si_code:         %d — %s\n", info->si_code,
           si_code_nombre(info->si_signo, info->si_code));

    printf("  si_pid:          %d (proceso origen)\n", (int)info->si_pid);
    printf("  si_uid:          %u (uid real del origen)\n", info->si_uid);

    if (info->si_signo == SIGCHLD) {
        printf("  si_status:       %d (exit code o número de señal)\n",
               info->si_status);
        printf("  si_utime:        %ld\n", (long)info->si_utime);
        printf("  si_stime:        %ld\n", (long)info->si_stime);
    }

    if (info->si_signo == SIGSEGV || info->si_signo == SIGBUS
        || info->si_signo == SIGFPE || info->si_signo == SIGILL) {
        printf("  si_addr:         %p (dirección del fault)\n", info->si_addr);
    }

    fflush(stdout);
}

int main(void)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = al_recibir;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);

    printf("PID=%d — Ctrl+C, kill -TERM/-HUP; compara si_pid/si_code\n",
           (int)getpid());
    printf("(kill -9 no se captura)\n");

    while (1)
        pause();

    return 0;
}
