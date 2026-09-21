/*
 * cat_bare.s — minimal cat in x86-64 Linux assembly (AT&T syntax)
 *
 * Build:  as -o cat_bare.o cat_bare.s && ld -o cat_bare cat_bare.o
 * Run:    ./cat_bare < file.txt
 *         echo "hola" | ./cat_bare
 */

    .section .bss
    .lcomm buffer, 4096

    .section .text
    .globl _start

_start:
loop:
    mov     $0, %rax
    mov     $0, %rdi
    lea     buffer(%rip), %rsi
    mov     $4096, %rdx
    syscall

    test    %rax, %rax
    jle     exit

    mov     %rax, %rdx
    mov     $1, %rax
    mov     $1, %rdi
    lea     buffer(%rip), %rsi
    syscall

    jmp     loop

exit:
    mov     $60, %rax
    xor     %rdi, %rdi
    syscall
