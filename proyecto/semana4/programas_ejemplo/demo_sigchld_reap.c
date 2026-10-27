/*
 * demo_sigchld_reap.c — recolección de hijos vía SIGCHLD (no polling en el loop principal)
 *
 * Compilar: gcc -Wall -Wextra demo_sigchld_reap.c -o demo_sigchld_reap
 * Ejecutar: ./demo_sigchld_reap
 *
 * strace: strace -e fork,wait4,sigaction ./demo_sigchld_reap
 */
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

static void reap_all(int signo)
{
    int saved_errno = errno;
    int status;

    (void)signo;
    /*
     * SIGCHLD puede llegar una vez aunque varios hijos hayan terminado;
     * waitpid(-1, ..., WNOHANG) en bucle vacía la cola de zombies.
     */
    while (waitpid(-1, &status, WNOHANG) > 0)
        ;
    errno = saved_errno;
}

int main(void)
{
    struct sigaction sa;

    sa.sa_handler = reap_all;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; /* opcional: reinicia syscalls interrumpidas */
    if (sigaction(SIGCHLD, &sa, NULL) < 0) {
        perror("sigaction");
        return 1;
    }

    fprintf(stderr, "[padre] pid=%d — fork cada 2 s; SIGCHLD recolecta\n", (int)getpid());

    for (int i = 0; i < 5; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return 1;
        }
        if (pid == 0) {
            sleep(1);
            _exit(42);
        }
        sleep(2);
        fprintf(stderr, "[padre] hijo %d lanzado; revisa: ps -o pid,stat --ppid %d\n",
                (int)pid, (int)getpid());
    }

    fprintf(stderr, "[padre] esperando 3 s finales (deberían no quedar Z)\n");
    sleep(3);
    return 0;
}
