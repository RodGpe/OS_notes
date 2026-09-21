/*
 * cat_intel.s — minimal cat in x86-64 Linux assembly (Intel syntax)
 *
 * Build:  as -o cat_intel.o cat_intel.s && ld -o cat_intel cat_intel.o
 * Run:    ./cat_intel < file.txt
 *         echo "hola" | ./cat_intel
 */

    .intel_syntax noprefix

    .section .bss
    .lcomm buffer, 4096

    .section .text
    .globl _start

_start:
loop:
    /* read(0, buffer, sizeof(buffer)) */
    mov     rax, 0              /* syscall: read */
    mov     rdi, 0              /* fd = stdin */
    /* lea rsi, buffer[rip]
     * LEA (Load Effective Address): calcula la direccion de buffer y la guarda en rsi.
     * No lee memoria; solo obtiene el puntero. rsi = 2do arg de read() (donde escribir).
     * buffer[rip]: direccionamiento relativo al contador de instrucciones (PIC en x86-64).
     */
    lea     rsi, buffer[rip]
    mov     rdx, 4096
    syscall

    test    rax, rax
    jle     exit                /* EOF or error */

    /* write(1, buffer, n) */
    mov     rdx, rax            /* n bytes read */
    mov     rax, 1              /* syscall: write */
    mov     rdi, 1              /* fd = stdout */
    lea     rsi, buffer[rip]        /* mismo puntero al buffer; 2do arg de write() */
    syscall

    jmp     loop

exit:
    /* exit(0) */
    mov     rax, 60             /* syscall: exit */
    xor     rdi, rdi            /* status = 0 */
    syscall
