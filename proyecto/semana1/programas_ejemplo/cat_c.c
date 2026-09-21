/*
 * cat_c.c — version sencilla del comando cat
 *
 * Compilar DINAMICO:  gcc -Wall -Wextra -O2 cat_c.c -o cat_c_dynamic
 * Compilar ESTATICO: gcc -Wall -Wextra -O2 -static cat_c.c -o cat_c_static
 * Comparar:           ls -lh cat_c_dynamic cat_c_static
 */
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

static int cat(int fd)
{
    char buf[4096];
    ssize_t n;

    while ((n = read(fd, buf, sizeof buf)) > 0)
        if (write(STDOUT_FILENO, buf, n) != n)
            return -1;
    return n < 0 ? -1 : 0;
}

/*
 * int main(int argc, char **argv)
 *
 * Punto de entrada del programa. El kernel/shell llama a main() al ejecutar el binario.
 *
 * int          — tipo de retorno: codigo de salida del proceso (0 = exito, != 0 = error).
 *                Ese valor lo recibe el shell (echo $?) o el proceso padre con wait().
 *
 * argc         — "argument count": cantidad de strings en argv (siempre >= 1).
 *                Incluye argv[0], el nombre con que se invoco el programa.
 *                Ejemplo: ./cat_c_dynamic foo.txt bar.txt  ->  argc == 3
 *
 * char **argv  — "argument vector": arreglo de punteros a char (C-strings).
 *                argv[0] = nombre del programa (ej. "./cat_c_dynamic")
 *                argv[1] = primer argumento del usuario (ej. "foo.txt")
 *                argv[2] = segundo argumento, etc.
 *                argv[argc] == NULL  (convencion: marca el final del arreglo)
 *
 * char **argv es equivalente a char *argv[]: arreglo de cadenas pasado por el shell.
 */
int main(int argc, char **argv)
{
    if (argc == 1)
        return cat(STDIN_FILENO) != 0;

    for (int i = 1; i < argc; i++) {
        int fd = open(argv[i], O_RDONLY);
        if (fd < 0 || cat(fd) < 0) {
            perror(argv[i]);
            return 1;
        }
        close(fd);
    }
    return 0;
}

//char *s = "hola";

// s  -> dirección donde empieza "hola"
// *s -> 'h'
// s[0] == *s