/*
 * demo_fork_basic.c — fork() crea un proceso hijo
 *
 * fork() duplica el proceso: dos instancias continuan tras la llamada.
 * Retorno: 0 en el hijo, PID del hijo en el padre, -1 en error.
 *
 * Compilar: gcc -Wall -Wextra demo_fork_basic.c -o demo_fork_basic
 * Ejecutar: ./demo_fork_basic
 * Observar: strace -e fork,getpid,write ./demo_fork_basic
 */
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        /* Hijo: fork() devolvio 0; getpid() es el PID propio del hijo. */
        dprintf(STDERR_FILENO, "[hijo]  pid=%d ppid=%d\n",
                (int)getpid(), (int)getppid());
    } else {
        /* Padre: fork() devolvio el PID del hijo recien creado. */
        dprintf(STDERR_FILENO, "[padre] pid=%d hijo=%d\n",
                (int)getpid(), (int)pid);
    }

    return 0;
}
