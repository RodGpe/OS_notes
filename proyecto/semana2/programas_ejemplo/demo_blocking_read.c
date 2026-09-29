/*
 * demo_blocking_read.c — read bloqueante en stdin
 *
 * Por defecto read() es bloqueante: si no hay datos, el proceso pasa a
 * estado BLOCKED y el scheduler ejecuta otros procesos. No hay busy-wait.
 *
 * Compilar: gcc -Wall -Wextra demo_blocking_read.c -o demo_blocking_read
 *
 * Terminal 1 — dejar el proceso bloqueado en read(0):
 *   ./demo_blocking_read
 *
 * Terminal 2 — estado del proceso (S = sleeping / blocked en I/O):
 *   (pgrep sin -f busca "comm", truncado a 15 chars: demo_blocking_re)
 *   PID=$(pgrep -n -f demo_blocking_read)
 *   ps -o pid,state,pcpu,wchan:30,cmd -p "$PID"
 *   cat /proc/$PID/status | grep -E '^(Name|State|Pid):'
 * Alternativa: ps aux | grep '[d]emo_blocking_read'
 *
 * Opcional — syscalls (read no retorna hasta que escribas en T1):
 *   strace -e read,write ./demo_blocking_read
 *
 * Con pipe (read desbloquea al instante):
 *   echo hola | ./demo_blocking_read
 *
 * Contraste (polling, quema CPU): demo_polling_read.c
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
    char buf[128];
    ssize_t n;

    write(STDERR_FILENO,
          "Esperando entrada (read bloqueante en stdin)...\n", 49);

    /*
     * read(0, ...) bloquea hasta que la terminal (o un pipe) entregue bytes.
     * Mientras tanto: este proceso no consume CPU en un while vacio.
     * El kernel despierta al proceso cuando hay datos listos en fd 0.
     *
     * Contraste pedagogico:
     *   while (1) { }     -> RUNNING, quema CPU
     *   read(fd, ...)     -> BLOCKED si no hay datos, eficiente
     */
    n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
    if (n < 0) {
        perror("read");
        return 1;
    }

    buf[n] = '\0';
    dprintf(STDERR_FILENO, "Leí %zd bytes: %s", n, buf);

    return 0;
}
