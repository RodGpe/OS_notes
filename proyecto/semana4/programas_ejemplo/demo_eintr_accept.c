/*
 * demo_eintr_accept.c — accept() y EINTR cuando una señal interrumpe la syscall
 *
 * Compilar: gcc -Wall -Wextra demo_eintr_accept.c -o demo_eintr_accept
 * Terminal 1: ./demo_eintr_accept
 * Terminal 2: kill -INT <pid>  (o Ctrl+C en T1 si está en foreground)
 *
 * Sin SA_RESTART, accept puede fallar con EINTR; el servidor debe reintentar.
 */
#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static volatile sig_atomic_t g_stop = 0;

static void on_sigint(int signo)
{
    (void)signo;
    g_stop = 1;
    fprintf(stderr, "\n[SIGINT] g_stop=1\n");
}

int main(void)
{
    struct sigaction sa;
    int listen_fd;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    sa.sa_handler = on_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; /* deliberado: NO SA_RESTART para ver EINTR en accept */
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror("sigaction");
        return 1;
    }

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return 1;
    }

    {
        int opt = 1;
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9099);

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }
    if (listen(listen_fd, 4) < 0) {
        perror("listen");
        return 1;
    }

    fprintf(stderr, "Escuchando :9099 pid=%d — envía SIGINT mientras accept bloquea\n",
            (int)getpid());

    while (!g_stop) {
        int client_fd = accept(listen_fd, (struct sockaddr *)&addr, &len);
        if (client_fd < 0) {
            if (errno == EINTR) {
                fprintf(stderr, "accept: EINTR (señal recibida), reintentando...\n");
                continue;
            }
            perror("accept");
            break;
        }
        fprintf(stderr, "Cliente conectado fd=%d\n", client_fd);
        close(client_fd);
    }

    close(listen_fd);
    fprintf(stderr, "Cierre ordenado.\n");
    return 0;
}
