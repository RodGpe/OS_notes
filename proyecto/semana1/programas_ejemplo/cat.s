/*
 * cat.s — minimal cat in x86-64 Linux assembly (AT&T syntax)
 *
 * Build:  as -o cat.o cat.s && ld -o cat cat.o
 * Run:    ./cat < file.txt
 *         echo "hola" | ./cat
 */

    .section .bss
    .lcomm buffer, 4096

    .section .text
    .globl _start

_start:
loop:
    /* read(0, buffer, sizeof(buffer)) */
    mov     $0, %rax            /* syscall: read */
    mov     $0, %rdi            /* fd = stdin */
    /* lea buffer(%rip), %rsi
     * LEA (Load Effective Address): calcula la direccion de buffer y la guarda en %rsi.
     * No lee memoria; solo obtiene el puntero. %rsi = 2do arg de read() (donde escribir).
     * buffer(%rip): direccionamiento relativo al contador de instrucciones (PIC en x86-64).
     */
    lea     buffer(%rip), %rsi
    mov     $4096, %rdx
    syscall

    test    %rax, %rax
    jle     exit                /* EOF or error */

    /* write(1, buffer, n) */
    mov     %rax, %rdx          /* n bytes read */
    mov     $1, %rax            /* syscall: write */
    mov     $1, %rdi            /* fd = stdout */
    lea     buffer(%rip), %rsi    /* mismo puntero al buffer; 2do arg de write() */
    syscall

    jmp     loop

exit:
    /* exit(0) */
    mov     $60, %rax           /* syscall: exit */
    xor     %rdi, %rdi          /* status = 0 */
    syscall
