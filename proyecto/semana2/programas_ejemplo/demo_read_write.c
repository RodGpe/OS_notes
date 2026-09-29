/*
 * demo_read_write.c — copiar un archivo con read/write/close
 *
 * Demuestra el patron basico de I/O en Unix: open devuelve un fd;
 * read/write mueven bytes entre user space y el kernel (page cache, disco);
 * close libera la entrada en la tabla del proceso.
 *
 * MiniServ usara el mismo patron sobre sockets en lugar de archivos.
 *
 * Compilar: gcc -Wall -Wextra demo_read_write.c -o demo_read_write
 * Ejecutar: ./demo_read_write /etc/hostname /tmp/copia-hostname
 * Observar: strace -e openat,read,write,close ./demo_read_write /etc/hostname /tmp/x
 */
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define BUF_SIZE 4096

int main(int argc, char *argv[])
{
    int in_fd, out_fd;
    char buf[BUF_SIZE];
    ssize_t n;

    if (argc != 3) {
        fprintf(stderr, "uso: %s <origen> <destino>\n", argv[0]);
        return 1;
    }

    /*
     * open() pide al kernel abrir el path y devuelve el siguiente fd libre
     * del proceso (tipicamente 3 si 0/1/2 estan ocupados).
     * El kernel mantiene un offset de lectura/escritura por fd abierto.
     */
    in_fd = open(argv[1], O_RDONLY);
    if (in_fd < 0) {
        perror("open origen");
        return 1;
    }

    out_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd < 0) {
        perror("open destino");
        close(in_fd);
        return 1;
    }

    /*
     * read() puede devolver menos bytes que los pedidos; hay que usar el
     * valor de retorno. Retorno 0 = EOF (no quedan datos en este fd).
     * Los bytes pasan: disco/page cache -> kernel -> buffer en user space.
     */
    while ((n = read(in_fd, buf, sizeof(buf))) > 0) {
        ssize_t off = 0;

        /*
         * write() tampoco garantiza escribir todo de una vez (partial write).
         * El bucle interno es buena practica; en Semana 14 lo retomamos.
         */
        while (off < n) {
            ssize_t w = write(out_fd, buf + off, (size_t)(n - off));
            if (w < 0) {
                perror("write");
                close(in_fd);
                close(out_fd);
                return 1;
            }
            off += w;
        }
    }

    if (n < 0)
        perror("read");

    /*
     * close() elimina la fila fd de la tabla del proceso y decrementa
     * la referencia del kernel al objeto (archivo). Sin close, fd leak.
     */
    close(in_fd);
    close(out_fd);
    return 0;
}
