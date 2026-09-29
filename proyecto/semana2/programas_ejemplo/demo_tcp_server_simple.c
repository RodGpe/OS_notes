/*
 * demo_tcp_server_simple.c — paso 1: socket, bind, listen, accept
 *
 * Version minima para ver primero en clase. Objetivo: entender que un socket
 * es un file descriptor y que el servidor necesita cuatro syscalls antes de
 * "tener un cliente". accept() bloquea hasta que alguien se conecte.
 *
 * Este demo NO lee del cliente: solo envia un mensaje fijo y termina.
 * Siguiente paso: demo_tcp_server.c (read + echo en loop).
 *
 * Compilar: gcc -Wall -Wextra demo_tcp_server_simple.c -o demo_tcp_server_simple
 * Ejecutar: ./demo_tcp_server_simple
 * Cliente:  nc 127.0.0.1 9090
 * Observar: strace -e socket,bind,listen,accept,write,close ./demo_tcp_server_simple
 *           ss -ltnp | grep 9090    (en otra terminal, mientras espera en accept)
 */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 9090

int main(void)
{
    int listen_fd, client_fd;
    struct sockaddr_in addr;
    const char msg[] = "MiniServ demo: conexion aceptada.\n";

    /* 1. Pedir al kernel un fd de socket (aun sin puerto ni conexion). */
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    /* 2. Asociar puerto local 9090 a ese fd. */
    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listen_fd);
        return 1;
    }

    /*
     * 3. Marcar como socket pasivo (servidor). Cola de 1 es suficiente
     *    para este demo de una sola conexion.
     */
    if (listen(listen_fd, 1) < 0) {
        perror("listen");
        close(listen_fd);
        return 1;
    }

    dprintf(STDERR_FILENO,
            "Escuchando en puerto %d (listen_fd=%d). Conecte con: nc 127.0.0.1 %d\n",
            PORT, listen_fd, PORT);

    /*
     * 4. Bloquear hasta que llegue un cliente. Devuelve OTRO fd (client_fd).
     *    Pasamos NULL en addr: no nos importa la IP del cliente en este paso.
     */
    client_fd = accept(listen_fd, NULL, NULL);
    if (client_fd < 0) {
        perror("accept");
        close(listen_fd);
        return 1;
    }

    dprintf(STDERR_FILENO, "Cliente conectado -> client_fd=%d\n", client_fd);

    /*
     * 5. Escribir al cliente por client_fd (no por listen_fd).
     *    Un solo write con mensaje fijo; sin read() todavia.
     */
    if (write(client_fd, msg, sizeof(msg) - 1) < 0)
        perror("write");

    close(client_fd);
    close(listen_fd);
    return 0;
}
