/*
 * demo_polling_read.c — contraste con demo_blocking_read.c (polling + read no bloqueante)
 *
 * En lugar de bloquearse en read(), el proceso pregunta una y otra vez si hay datos
 * (polling). Con O_NONBLOCK, read() devuelve -1 y errno EAGAIN si aun no hay bytes.
 * El proceso sigue en RUNNING y consume CPU — malo para esperar I/O, util para ver
 * la diferencia frente al I/O bloqueante.
 *
 * Comparar en dos terminales:
 *   demo_blocking_read.c  -> ps: state S (sleeping)
 *   demo_polling_read.c   -> ps: state R (running), CPU sube en top
 *
 * Compilar: gcc -Wall -Wextra demo_polling_read.c -o demo_polling_read
 *
 * Terminal 1:
 *   ./demo_polling_read
 *
 * Terminal 2:
 *   PID=$(pgrep -n -f demo_polling_read)
 *   ps -o pid,state,pcpu,cmd -p "$PID"
 *   top -p "$PID"    # ver %CPU mientras espera en T1
 *
 * strace (muchas read fallidas con EAGAIN):
 *   strace -e read ./demo_polling_read
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
    char buf[128];
    ssize_t n;
    unsigned long polls = 0;
    int flags;

    write(STDERR_FILENO,
          "Esperando entrada (polling en stdin, O_NONBLOCK)...\n", 54);

    /*
     * Sin esto, read() bloquearia igual que demo_blocking_read.c.
     * O_NONBLOCK: "si no hay datos, no duermas; devuelve EAGAIN".
     */
    flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags < 0) {
        perror("fcntl F_GETFL");
        return 1;
    }
    if (fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("fcntl O_NONBLOCK");
        return 1;
    }

    /*
     * Bucle de polling: el proceso no cede el CPU al quedarse blocked en read.
     * Cada vuelta puede ser una syscall read (costosa si hay millones de vueltas).
     *
     * Contraste:
     *   demo_blocking_read  -> una read, proceso en S hasta que hay datos
     *   demo_polling_read     -> muchas read/EAGAIN, proceso en R quemando CPU
     */
    for (;;) {
        n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
        if (n > 0)
            break;
        if (n == 0)
            break; /* EOF (p. ej. pipe cerrado sin datos) */
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                polls++;
                continue;
            }
            perror("read");
            return 1;
        }
    }

    if (n <= 0) {
        dprintf(STDERR_FILENO, "Sin datos (EOF). Intentos de polling: %lu\n",
                polls);
        return 0;
    }

    buf[n] = '\0';
    dprintf(STDERR_FILENO, "Leí %zd bytes tras %lu intentos de polling: %s",
            n, polls, buf);

    return 0;
}
