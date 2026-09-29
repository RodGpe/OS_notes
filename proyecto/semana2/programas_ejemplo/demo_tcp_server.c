/*
 * demo_tcp_server.c — paso 2: servidor TCP con read/echo
 *
 * Ver primero: demo_tcp_server_simple.c (solo socket/bind/listen/accept
 * y un mensaje fijo). Este archivo agrega read/write en loop (echo).
 *
 * En Unix "todo es un archivo": un socket es otro tipo de objeto expuesto
 * como file descriptor. Las mismas syscalls read/write/close aplican.
 *
 * Secuencia servidor: socket -> bind -> listen -> accept -> read/write.
 * accept() es bloqueante si no hay conexiones pendientes.
 *
 * Compilar: gcc -Wall -Wextra demo_tcp_server.c -o demo_tcp_server
 * Ejecutar: ./demo_tcp_server
 * Cliente:  echo hola | nc 127.0.0.1 9090
 * Observar: strace -e socket,bind,listen,accept,read,write,close ./demo_tcp_server
 *           ss -ltnp | grep 9090
 *           ls -l /proc/<pid>/fd   (socket:[...] para fds de red)
 *           
 */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 9090
#define BACKLOG 4

static void die(const char *msg)
{
    perror(msg);
    _exit(1);
}

int main(void)
{
    int listen_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    char buf[256];
    ssize_t n;

    /*
     * socket() crea un endpoint en el kernel y devuelve un fd.
     * Aun no hay puerto asignado ni conexiones; solo el objeto socket.
     */
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
        die("socket");

    /*
     * SO_REUSEADDR evita "Address already in use" al reiniciar rapido el demo.
     * Es opcion a nivel socket administrada por el kernel, no concepto central.
     */
    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        die("setsockopt");

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);  /* escuchar en todas las interfaces */
    addr.sin_port = htons(PORT);               /* puerto en orden de red (big-endian) */

    /*
     * bind() asocia direccion IP + puerto local al listen_fd.
     * El puerto es recurso del sistema: solo un listener activo por (IP, puerto).
     */
    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        die("bind");

    /*
     * listen() marca el socket como pasivo (servidor) y crea cola de
     * conexiones pendientes (BACKLOG). Aun no hay clientes conectados.
     */
    if (listen(listen_fd, BACKLOG) < 0)
        die("listen");

    dprintf(STDERR_FILENO, "listen_fd=%d\n", listen_fd);

    /*
     * accept() bloquea hasta que un cliente complete el three-way handshake TCP.
     * Devuelve un NUEVO fd (client_fd) para hablar con ese cliente.
     * listen_fd sigue abierto para futuros accept() — clave en servidores reales.
     *
     * Error frecuente: read(listen_fd) para obtener datos del cliente.
     * Los datos van por client_fd, no por listen_fd.
     */
    client_fd = accept(listen_fd, (struct sockaddr *)&addr, &addrlen);
    if (client_fd < 0)
        die("accept");

    dprintf(STDERR_FILENO, "accept -> client_fd=%d\n", client_fd);

    {
        ssize_t total = 0;

        /*
         * TCP es un byte stream: un mensaje puede llegar en varios read()
         * o varios mensajes en un solo read(). Solo importa el retorno de read.
         * Retorno 0 = peer cerro su lado de la conexion (FIN).
         */
        while ((n = ) > 0) {
            total += n;read(client_fd, buf, sizeof(buf))
            ssize_t off = 0;
            while (off < n) {
                ssize_t w = write(client_fd, buf + off, (size_t)(n - off));
                if (w < 0)
                    die("write");
                off += w;
            }
        }

        if (n < 0)
            perror("read");

        dprintf(STDERR_FILENO, "read %zd bytes total, echo OK\n", total);
    }

    close(client_fd);   /* libera solo esta conexion */
    close(listen_fd);   /* en MiniServ V1 listen_fd permanece abierto en un loop */
    return 0;
}

// commandos para observar

// ss -ltnp | grep 9090  // Socket Statistics