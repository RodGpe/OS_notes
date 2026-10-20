/*
 * demo_zombie.c — proceso zombie: hijo terminado, padre sin wait()
 *
 * Cuando un hijo termina, su entrada en la tabla de procesos queda como
 * ZOMBIE (estado Z) hasta que el padre llame wait/waitpid y lea el exit status.
 * El hijo ya no ejecuta codigo; el kernel guarda metadata para el padre.
 *
 * Este demo NO hace wait() durante 20 s a proposito, para ver la Z en ps.
 *
 * Compilar: gcc -Wall -Wextra demo_zombie.c -o demo_zombie
 *
 * Terminal 1:
 *   ./demo_zombie
 *
 * Terminal 2 (mientras el padre duerme; usa el PID del hijo que imprime T1):
 *   ps -o pid,ppid,stat,cmd -p <pid_hijo>
 *   ps -o pid,ppid,stat,cmd --ppid <pid_padre>
 *
 * Esperado: STAT = Z o Z+ ; CMD como [demo_zombie] <defunct>
 *
 * strace:
 *   strace -e fork,wait4,sleep ./demo_zombie
 *
 * Al final el padre hace wait() y el zombie desaparece de ps.
 *
 * Contraste: demo_fork_wait.c (wait inmediato, sin Z persistente).
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
         * Hijo termina de inmediato. Tras _exit no hay codigo del hijo corriendo.
         * Sin wait en el padre, el kernel conserva slot ZOMBIE con este PID.
         */
        dprintf(STDERR_FILENO,
                "[hijo] pid=%d termino; si el padre no hace wait, quedo zombie\n",
                (int)getpid());
        _exit(0);
    }

    /*
     * Padre: fork() devolvio el PID del hijo. El hijo puede estar ya en Z
     * mientras el padre sigue ejecutando (sleep abajo).
     */
    dprintf(STDERR_FILENO,
            "[padre] pid=%d, hijo=%d — NO hago wait durante 20 s\n",
            (int)getpid(), (int)pid);
    dprintf(STDERR_FILENO,
            "[padre] en otra terminal: ps -o pid,ppid,stat,cmd -p %d\n",
            (int)pid);

    sleep(20);

    dprintf(STDERR_FILENO, "[padre] ahora llamo wait() para recolectar al hijo\n");
    if (wait(NULL) < 0)
        perror("wait");
    else
        dprintf(STDERR_FILENO,
                "[padre] wait() OK — el zombie ya no debe aparecer en ps\n");

    return 0;
}
