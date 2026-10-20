/*
 * demo_fork_wait.c — padre espera al hijo con wait()
 *
 * wait() bloquea hasta que un hijo termina; evita zombie (estado Z).
 *
 * Compilar: gcc -Wall -Wextra demo_fork_wait.c -o demo_fork_wait
 * Ejecutar: ./demo_fork_wait
 * Observar: strace -e fork,wait4,getpid ./demo_fork_wait
 *           (durante sleep del hijo: ps -o pid,stat,cmd -p <hijo>  -> S)
 *           (sin wait: ps mostraria Z al terminar el hijo)
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
        dprintf(STDERR_FILENO, "[hijo] pid=%d, duermo 2s...\n", (int)getpid());
        sleep(10);
        dprintf(STDERR_FILENO, "[hijo] termino\n");
        return 0;
    }

    dprintf(STDERR_FILENO, "[padre] esperando hijo %d con wait()...\n", (int)pid);
    if (wait(NULL) < 0)
        perror("wait");
    dprintf(STDERR_FILENO, "[padre] hijo recolectado\n");

    return 0;
}
