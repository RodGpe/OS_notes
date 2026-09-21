/*
 * demo_fd.c — file descriptors en Linux
 *
 * Un fd (file descriptor) es un entero >= 0: un indice en la tabla del proceso.
 * El kernel usa ese numero para saber a que recurso apunta (terminal, archivo, etc.).
 *
 * Compilar: gcc -Wall -Wextra demo_fd.c -o demo_fd
 * Observar: strace -e openat,read,write,close ./demo_fd
 */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
    const char *path = "/tmp/so-demo.txt";
    const char *msg = "guardado en el archivo\n";
    char buf[64];
    ssize_t n;
    int fd;

    /* Todo proceso hereda 3 fds al nacer: 0=stdin, 1=stdout, 2=stderr */
    //ejemplo ls -l | grep '\.c$' | wc -l
    printf("fds estandar: stdin=%d  stdout=%d  stderr=%d\n",
           STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
    fflush(stdout);  /* write() no usa el buffer de printf; vaciamos antes */

    /*
     * open() pide al kernel abrir un archivo y devuelve el siguiente fd libre.
     * No devuelve la ruta: solo un entero que usaremos en read/write/close.
     */
    fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0644); 
    if (fd < 0) {
        perror("open");
        return 1;
    }
    printf("open(\"%s\") devolvio fd=%d\n", path, fd);

    /* Misma syscall (write), distintos fds -> distintos destinos */
    write(STDOUT_FILENO, "esto va a fd 1 (stdout)\n", 24);
    write(STDERR_FILENO, "esto va a fd 2 (stderr)\n", 24);
    write(fd, msg, strlen(msg));  /* fd del archivo, no 1 ni 2 */

    close(fd);  /* liberamos el fd; el kernel puede reutilizar ese numero */

    /* Reabrimos solo para leer: demostramos read() sobre el mismo recurso */
    fd = open(path, O_RDONLY);
    n = read(fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("read(fd=%d) devolvio: %s", fd, buf);
    }

    close(fd);
    return 0;
}
