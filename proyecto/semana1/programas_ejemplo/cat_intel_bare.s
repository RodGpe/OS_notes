/*
 * cat_intel_bare.s — minimal cat in x86-64 Linux assembly (Intel syntax)
 *
 * Build:  as -o cat_intel_bare.o cat_intel_bare.s && ld -o cat_intel_bare cat_intel_bare.o
 * Run:    ./cat_intel_bare < file.txt
 *         echo "hola" | ./cat_intel_bare
 */

    .intel_syntax noprefix

    .section .bss
    .lcomm buffer, 4096

    .section .text
    .globl _start

_start:
loop:
    mov     rax, 0
    mov     rdi, 0
    lea     rsi, buffer[rip]
    mov     rdx, 4096
    syscall

    test    rax, rax
    jle     exit

    mov     rdx, rax
    mov     rax, 1
    mov     rdi, 1
    lea     rsi, buffer[rip]
    syscall

    jmp     loop

exit:
    mov     rax, 60
  /*  xor     rdi, rdi */
    mov      rdi, 1
    syscall


