/*
 * demo_sigsegv_catch.c — capturar SIGSEGV y continuar (solo didáctico)
 *
 * Demostración rápida — cambiar solo DEMO_CON_HANDLER y recompilar:
 *
 *   DEMO_CON_HANDLER 1  → BLOQUE A: handler, siglongjmp, "Fin normal"
 *   DEMO_CON_HANDLER 0  → BLOQUE B: *p = 42 sin handler → Segmentation fault
 *
 * Compilar:
 *   gcc -Wall -Wextra demo_sigsegv_catch.c -o demo_sigsegv_catch
 */
#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#define DEMO_CON_HANDLER 1 /* ← poner 0 para la demo que termina en SIGSEGV */

#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#if DEMO_CON_HANDLER

static sigjmp_buf g_jump;

static void on_sigsegv(int signo, siginfo_t *info, void *ucontext)
{
    (void)ucontext;

    fprintf(stderr,
            "\n[handler] SIGSEGV (%d) — si_addr=%p si_code=%d\n",
            signo, info->si_addr, info->si_code);
    fprintf(stderr, "[handler] siglongjmp: no reintentamos la instrucción inválida\n");
    fflush(stderr);

    siglongjmp(g_jump, 1);
}

/*
 * setup_sigsegv — registra en el kernel qué hacer ante un fallo de memoria (SIGSEGV).
 *
 * Sin esta función, un acceso inválido (p. ej. *NULL) usa la acción por defecto:
 * el kernel termina el proceso (a menudo "Segmentation fault" + core).
 *
 * Con sigaction(SIGSEGV, ...):
 *   - el kernel llama a on_sigsegv() en lugar de matar al proceso de inmediato;
 *   - SA_SIGINFO pasa siginfo_t (dirección si_addr, si_code, etc.) al handler;
 *   - sa_mask vacío: no bloqueamos otras señales extra durante el handler.
 *
 * El handler no "arregla" la memoria: solo informa y siglongjmp() salta a un
 * sigsetjmp() en main (ver BLOQUE A). Si no hubiera ese salto, volver a la
 * instrucción que falló volvería a disparar SIGSEGV.
 */
static void setup_sigsegv(void)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = on_sigsegv;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;

    if (sigaction(SIGSEGV, &sa, NULL) < 0) {
        perror("sigaction SIGSEGV");
        _exit(1);
    }
}

#endif /* DEMO_CON_HANDLER */

int main(void)
{
    int *p = NULL;

    printf("PID=%d\n", (int)getpid());
    fflush(stdout);

    printf("Antes del acceso inválido (*p = 42 con p=NULL)...\n");
    fflush(stdout);

#if DEMO_CON_HANDLER /* ====== BLOQUE A — CON handler ====== */

    setup_sigsegv();

    if (sigsetjmp(g_jump, 1) == 0) {
        *p = 42;
        printf("Esta línea no corre si hubo salto desde el handler\n");
    } else {
        printf("Volvimos aquí tras capturar SIGSEGV — el proceso sigue\n");
    }

#else /* ====== BLOQUE B — SIN handler ====== */

    *p = 42;
    printf("No verás esta línea: el kernel termina el proceso antes\n");

#endif /* DEMO_CON_HANDLER */

    printf("Fin normal del programa (return 0)\n");
    return 0;
}
