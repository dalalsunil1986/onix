%include "boot.inc"

[bits 32]
section .text

extern main

global _start
_start:
    mov byte [gs:0], 'K'
    call main

halt:
    xchg bx, bx;
    jmp halt
