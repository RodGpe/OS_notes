/*
 * demo_sigint_default.c — comportamiento por defecto de SIGINT (Ctrl+C)
 *
 * Sin handler personalizado, SIGINT termina el proceso.
 * Compilar: gcc -Wall -Wextra demo_sigint_default.c -o demo_sigint_default
 * Ejecutar: ./demo_sigint_default
 *           (presionar Ctrl+C)
 */
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    fprintf(stderr, "PID=%d — presiona Ctrl+C (SIGINT, acción por defecto: terminar)\n",
            (int)getpid());
    for (;;)
        pause(); /* bloquea hasta una señal */
    return 0;
}
