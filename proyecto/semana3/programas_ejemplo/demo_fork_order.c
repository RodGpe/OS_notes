/*
 * demo_fork_order.c — prediccion: orden de impresion tras fork()
 *
 * Compilar: gcc -Wall -Wextra demo_fork_order.c -o demo_fork_order
 * Ejecutar: ./demo_fork_order   (varias veces)
 */
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    printf("A\n");
    fflush(stdout);

    if (fork() == 0) {
        printf("B (hijo)\n");
    } else {
        printf("C (padre)\n");
    }

    printf("D\n");
    return 0;
}
