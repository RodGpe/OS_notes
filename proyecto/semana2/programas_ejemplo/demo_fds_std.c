/*
 * demo_fds_std.c — stdin, stdout y stderr como file descriptors
 *
 * Un fd es un indice en la tabla de descriptores del proceso. El kernel
 * traduce ese entero al recurso real (terminal, archivo, socket, etc.).
 * Todo proceso hereda fd 0, 1 y 2 al nacer — convencion POSIX.
 *
 * Compilar: gcc -Wall -Wextra demo_fds_std.c -o demo_fds_std
 * Ejecutar: ./demo_fds_std
 * Observar: strace -e write ./demo_fds_std
 *           ls -l /proc/self/fd   (en otro proceso, usar el PID de ./demo_fds_std)
 */
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    char line[64];
    int len = snprintf(line, sizeof(line),
                       "stdin=%d stdout=%d stderr=%d\n",
                       STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);

    /*
     * write() es syscall directa: cruza user/kernel en cada llamada.
     * Mismo numero de syscall, distinto fd -> distinto objeto en el kernel.
     * fd 1 suele apuntar a la terminal; fd 2 tambien, pero stderr no se
     * mezcla con stdout cuando se redirige solo una de las dos (p. ej. 2>err.log).
     */
    write(STDOUT_FILENO, line, (size_t)len);

    write(STDOUT_FILENO, "esto va a stdout (fd 1)\n", 24);
    write(STDERR_FILENO, "esto va a stderr (fd 2)\n", 24);

    return 0;
}
