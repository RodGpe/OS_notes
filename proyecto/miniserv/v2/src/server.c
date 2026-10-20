/*
 * MiniServ V2 — HTTP secuencial (Semana 2+)
 *
 * Un proceso: accept -> leer peticion HTTP (bytes) -> responder HTTP -> close.
 * HTTP es solo formato de bytes sobre el mismo client_fd; el SO ve read/write.
 * Concurrencia limitada igual que V1; V3 anade fork-per-client.
 *
 * Compilar y arrancar (desde miniserv/v2/):
 *   make
 *   ./miniserv
 *
 * Probar en otra terminal (puerto PORT, ver #define abajo):
 *   curl -v http://127.0.0.1:8081/
 *   curl -v http://127.0.0.1:8081/hola
 *   curl http://127.0.0.1:8081/ruta/cualquiera
 *
 * Peticion HTTP minima con nc:
 *   printf 'GET /foo HTTP/1.1\r\nHost: localhost\r\n\r\n' | nc 127.0.0.1 8081
 *
 * Observar syscalls:
 *   strace -e accept,read,write,close ./miniserv
 *   ss -ltnp | grep 8081
 *
 * Si "Address already in use": otro miniserv ya usa el puerto
 *   ss -ltnp | grep 8081
 *
 * Detener: Ctrl+C en la terminal del servidor.
 */
#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8081
#define BACKLOG 8
#define BUF_SIZE 4096

static void die(const char *msg)
{
    perror(msg);
    _exit(1);
}

/*
 * write() puede transferir menos bytes de los pedidos (partial write).
 * Bucle hasta enviar todo — misma idea que en V1 y en servidores reales.
 */
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

/*
 * Lee hasta tener la cabecera HTTP completa (\r\n\r\n) o llenar el buffer.
 * TCP es byte stream: curl puede enviar la peticion en uno o varios read().
 * read() bloqueante: el proceso queda en S hasta que lleguen bytes del cliente.
 */
static ssize_t read_http_headers(int fd, char *buf, size_t cap)
{
    size_t total = 0;

    while (total < cap - 1) {
        ssize_t n = read(fd, buf + total, cap - 1 - total);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        if (n == 0)
            break; /* cliente cerro sin enviar cabecera completa */

        total += (size_t)n;
        buf[total] = '\0';

        if (strstr(buf, "\r\n\r\n") != NULL)
            break;
    }

    return (ssize_t)total;
}

/*
 * Parseo minimo en user space; no hay syscall extra.
 * Solo necesitamos la primera linea para el laboratorio (path del GET).
 */
static void extract_get_path(const char *req, char *path, size_t path_cap)
{
    const char *line_end = strstr(req, "\r\n");
    size_t line_len = line_end ? (size_t)(line_end - req) : strlen(req);

    if (path_cap > 0)
        path[0] = '/';

    if (line_len >= 4 && strncmp(req, "GET ", 4) == 0) {
        const char *start = req + 4;
        const char *end = memchr(start, ' ', line_len - 4);
        if (end && (size_t)(end - start) + 1 < path_cap) {
            memcpy(path, start, (size_t)(end - start));
            path[end - start] = '\0';
        }
    }
}

/*
 * Atiende un cliente ya aceptado: todo I/O ocurre sobre client_fd.
 * Connection: close — una peticion por conexion; luego main() hace close(fd).
 */
static void serve_http_client(int client_fd)
{
    char req[BUF_SIZE];
    char path[256];
    char body[512];
    char response[1024];
    ssize_t n;
    int body_len, resp_len;

    n = read_http_headers(client_fd, req, sizeof(req));
    if (n <= 0)
        return;

    extract_get_path(req, path, sizeof(path));

    body_len = snprintf(body, sizeof(body),
                        "MiniServ V2\nPath solicitado: %s\n", path);
    if (body_len < 0 || (size_t)body_len >= sizeof(body))
        return;

    /* Respuesta HTTP = bytes en el mismo stream TCP (write -> kernel -> NIC). */
    resp_len = snprintf(response, sizeof(response),
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/plain; charset=utf-8\r\n"
                        "Connection: close\r\n"
                        "Content-Length: %d\r\n"
                        "\r\n"
                        "%s",
                        body_len, body);
    if (resp_len < 0 || (size_t)resp_len >= sizeof(response))
        return;

    write_all(client_fd, response, (size_t)resp_len);
}

int main(void)
{
    int listen_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

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

    /* Log en stderr (fd 2): no mezclar con la respuesta HTTP en client_fd. */
    {
        char banner[] =
        //const char banner[] =
            "MiniServ V2 (HTTP secuencial) http://127.0.0.1:";
        char banner_puerto[200]="";
        sprintf(banner_puerto, "%s %d/\n",banner,PORT);
        if (write_all(STDERR_FILENO, banner_puerto, sizeof(banner_puerto) - 1) < 0)
            die("write");
    }

    /*
     * Un solo proceso: mientras serve_http_client() bloquea en read(),
     * no se llama accept() — segundo cliente espera en cola del kernel.
     */
    for (;;) {
        addrlen = sizeof(addr);
        client_fd = accept(listen_fd, (struct sockaddr *)&addr, &addrlen);
        if (client_fd < 0) {
            if (errno == EINTR)
                continue;
            die("accept");
        }

        serve_http_client(client_fd);
        close(client_fd); /* libera fd; listen_fd sigue para la siguiente vuelta */
    }
}
