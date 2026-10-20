/*
 * demo_exec_intro.c — introduccion a exec: el hijo se reemplaza por otro programa
 *
 * fork() crea copia; exec*() sustituye la imagen del proceso (mismo PID).
 * Solo tiene sentido en el hijo (o tras fork); el padre sigue ejecutando server.c.
 *
 * Compilar: gcc -Wall -Wextra demo_exec_intro.c -o demo_exec_intro
 * Ejecutar: ./demo_exec_intro
 * Observar: strace -e fork,execve,wait4 ./demo_exec_intro
 * Tabla exec*: ../familia-exec.md
 */
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        /*
         * Si execve tiene exito, no regresa. El codigo debajo no corre.
         * Reemplaza este proceso por /bin/echo (programa distinto, mismo PID).
         */
        execlp("echo", "echoo", "hola desde exec en el hijo", (char *)NULL);
        perror("exec");
        _exit(127);
    }

    wait(NULL);
    dprintf(STDERR_FILENO, "[padre] el hijo ya termino (ejecuto echo)\n");
    return 0;
}
