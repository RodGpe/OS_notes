/*
 * demo_byte_count.c — contar bytes leidos desde stdin
 *
 * Ejercicio de clase: practicar read() en loop y fd 0 (stdin).
 * Cada read(0, &c, 1) es una syscall; con pipes (echo x | ./demo)
 * el kernel entrega bytes desde el buffer del pipe al proceso.
 *
 * Compilar: gcc -Wall -Wextra demo_byte_count.c -o demo_byte_count
 * Ejecutar: echo abcde | ./demo_byte_count
 * Observar: strace -e read ./demo_byte_count  (muchas syscalls read de 1 byte)
 */
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    unsigned char c;
    ssize_t n;
    long count = 0;

    /*
     * read() retorna >0 (bytes leidos), 0 (EOF) o -1 (error).
     * Leer de a 1 byte es ineficiente (una syscall por byte) pero didactico.
     * En produccion se usa buffer grande (como demo_read_write.c).
     */
    while ((n = read(STDIN_FILENO, &c, 1)) > 0)
        count++;

    if (n < 0) {
        perror("read");
        return 1;
    }

    /* stderr (fd 2) para no mezclar diagnostico con datos en stdout */
    dprintf(STDERR_FILENO, "bytes leidos: %ld\n", count);
    return 0;
}
