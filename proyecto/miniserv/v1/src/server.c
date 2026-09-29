/*
 * MiniServ V1 — Semana 2
 * Servidor TCP secuencial: un proceso, loop accept -> echo -> close(client_fd).
 * listen_fd permanece abierto; sin fork ni threads.
 */
#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BACKLOG 8
#define BUF_SIZE 4096

static void die(const char *msg)
{
    perror(msg);
    _exit(1);
}

static ssize_t write_all(int fd, const void *buf, size_t count)
{
    const char *p = buf;
    size_t left = count;

    while (left > 0) {
        ssize_t w = write(fd, p, left);
        if (w < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        p += (size_t)w;
        left -= (size_t)w;
    }
    return (ssize_t)count;
}

int main(void)
{
    int listen_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    char buf[BUF_SIZE];
    ssize_t n;

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
        die("socket");

    {
        int opt = 1;
        if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
            die("setsockopt");
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        die("bind");
    if (listen(listen_fd, BACKLOG) < 0)
        die("listen");

    {
        const char banner[] = "MiniServ V1 escuchando en puerto 8080\n";
        if (write_all(STDERR_FILENO, banner, sizeof(banner) - 1) < 0)
            die("write");
    }

    /*
     * Loop principal: el proceso no termina tras el primer cliente.
     * accept() bloquea hasta la siguiente conexion (I/O bloqueante).
     */
    for (;;) {
        addrlen = sizeof(addr);
        client_fd = accept(listen_fd, (struct sockaddr *)&addr, &addrlen);
        if (client_fd < 0) {
            if (errno == EINTR)
                continue;
            die("accept");
        }

        while ((n = read(client_fd, buf, sizeof(buf))) > 0) {
            if (write_all(client_fd, buf, (size_t)n) < 0) {
                perror("write");
                break;
            }
        }
        if (n < 0)
            perror("read");

        close(client_fd);
    }
}
