/*
 * MiniServ V3 — Semana 3 (fork-per-client)
 *
 * Padre: accept -> fork -> cierra client_fd -> waitpid (WNOHANG) para no acumular zombies.
 * Hijo: cierra listen_fd, echo al cliente, _exit(0).
 */
#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define PORT 8080
#define BACKLOG 8
#define BUF_SIZE 4096

/*
 * Pausa didáctica al atender cada cliente (hijo en sleep/read).
 * Ver README — experimento hijos vivos bajo carga / apagado brusco vs V4.
 */
#define SERVE_DELAY_SEC 0

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

static void serve_client(int client_fd)
{
    char buf[BUF_SIZE];
    ssize_t n;

    if (SERVE_DELAY_SEC > 0) {
        dprintf(STDERR_FILENO, "[hijo pid=%d] pausa %d s antes del echo\n",
                (int)getpid(), SERVE_DELAY_SEC);
        sleep((unsigned int)SERVE_DELAY_SEC);
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

/*
 * Recolecta hijos que ya terminaron (evita procesos zombie, estado Z).
 *
 * Cuando un hijo hace _exit(0) tras atender un cliente, su PID sigue en la
 * tabla del kernel como ZOMBIE hasta que el padre llame wait/waitpid.
 * Sin recolectar, ps mostraria hijos <defunct> y se agotarian PIDs con el tiempo.
 *
 * waitpid(-1, ...)  -> cualquier hijo de este proceso
 * WNOHANG           -> no bloquear: si no hay zombie listo, retorna 0 y seguimos
 *                     a accept() (a diferencia de wait(), que dormiria al padre)
 * while (... > 0)   -> puede haber varios hijos que terminaron entre un accept y otro
 *
 * Ver: semana3/programas_ejemplo/demo_zombie.c
 */
static void reap_children(void)
{
    int status;

    while (waitpid(-1, &status, WNOHANG) > 0)
        ;
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

    dprintf(STDERR_FILENO,
            "MiniServ V3 (fork-per-client) en puerto %d, pid=%d\n",
            PORT, (int)getpid());

    for (;;) {
        pid_t pid;

        /* Limpiar hijos que murieron mientras atendiamos al cliente anterior. */
        reap_children();

        addrlen = sizeof(addr);
        client_fd = accept(listen_fd, (struct sockaddr *)&addr, &addrlen);
        if (client_fd < 0) {
            if (errno == EINTR)
                continue;
            die("accept");
        }

        pid = fork();
        if (pid < 0) {
            perror("fork");
            close(client_fd);
            continue;
        }

        if (pid == 0) {
            /* Hijo: no debe conservar listen_fd (evita fds duplicados innecesarios). */
            close(listen_fd);
            serve_client(client_fd);
            _exit(0);
        }

        /* Padre: no usa este client_fd; el hijo lo atiende. */
        close(client_fd);
        /*
         * Si el hijo termina muy rapido (cliente desconecto al instante),
         * recolectarlo aqui; si aun esta en read/write, reap_children() en la
         * siguiente vuelta del loop lo recogera antes del proximo accept.
         */
        reap_children();
    }
}
