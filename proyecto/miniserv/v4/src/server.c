/*
 * MiniServ V4 — Semana 5 (señales + cierre ordenado)
 *
 * Padre: sigaction(SIGINT/SIGTERM/SIGCHLD), accept -> fork, reap vía SIGCHLD.
 * Hijo: cierra listen_fd, echo, _exit(0).
 */
#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define PORT 8080
#define BACKLOG 8
#define BUF_SIZE 4096
#define MAX_CHILDREN 512

static volatile sig_atomic_t g_stop = 0;
static pid_t g_children[MAX_CHILDREN];
static size_t g_nchildren;

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

    while ((n = read(client_fd, buf, sizeof(buf))) > 0) {
        if (write_all(client_fd, buf, (size_t)n) < 0) {
            if (errno == EPIPE)
                break;
            perror("write");
            break;
        }
    }
    if (n < 0 && errno != EINTR)
        perror("read");
    close(client_fd);
}

static void track_child(pid_t pid)
{
    if (g_nchildren < MAX_CHILDREN)
        g_children[g_nchildren++] = pid;
}

static void reap_children(void)
{
    int status;

    while (waitpid(-1, &status, WNOHANG) > 0)
        ;
}

/*
 * SIGKILL (kill -9) al padre no se puede capturar: el kernel mata al padre al
 * instante y este código no corre. Ante SIGINT/SIGTERM el padre apaga y envía
 * SIGKILL a cada hijo registrado (p. ej. bloqueados en sleep/read).
 */
static void kill_all_children(void)
{
    size_t i;

    for (i = 0; i < g_nchildren; i++) {
        if (g_children[i] <= 0)
            continue;
        if (kill(g_children[i], SIGKILL) == 0)
            dprintf(STDERR_FILENO, "MiniServ V4: SIGKILL al hijo pid=%d\n",
                    (int)g_children[i]);
    }
}

static void on_stop(int signo)
{
    (void)signo;
    g_stop = 1;
}

static void on_sigchld(int signo)
{
    int saved_errno = errno;

    (void)signo;
    reap_children();
    errno = saved_errno;
}

static void setup_signals(void)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_stop;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) < 0)
        die("sigaction SIGINT");
    if (sigaction(SIGTERM, &sa, NULL) < 0)
        die("sigaction SIGTERM");

    sa.sa_handler = on_sigchld;
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) < 0)
        die("sigaction SIGCHLD");

    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if (sigaction(SIGPIPE, &sa, NULL) < 0)
        die("sigaction SIGPIPE");
}

static void shutdown_server(int listen_fd)
{
    int status;

    if (listen_fd >= 0)
        close(listen_fd);

    dprintf(STDERR_FILENO,
            "MiniServ V4: apagado ordenado, SIGKILL a hijos vivos...\n");

    kill_all_children();

    while (waitpid(-1, &status, 0) > 0)
        ;
}

int main(void)
{
    int listen_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    setup_signals();

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
            "MiniServ V4 (fork-per-client + señales) en puerto %d, pid=%d\n",
            PORT, (int)getpid());

    while (!g_stop) {
        pid_t pid;

        addrlen = sizeof(addr);
        client_fd = accept(listen_fd, (struct sockaddr *)&addr, &addrlen);
        if (client_fd < 0) {
            if (errno == EINTR) {
                if (g_stop)
                    break;
                continue;
            }
            die("accept");
        }

        pid = fork();
        if (pid < 0) {
            perror("fork");
            close(client_fd);
            continue;
        }

        if (pid == 0) {
            close(listen_fd);
            serve_client(client_fd);
            _exit(0);
        }

        close(client_fd);
        track_child(pid);
    }

    shutdown_server(listen_fd);

    {
        const char done[] = "MiniServ V4: terminado\n";
        (void)write_all(STDERR_FILENO, done, sizeof(done) - 1);
    }

    return 0;
}
