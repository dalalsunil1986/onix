%include "boot.inc"

[bits 32]
section .text

extern init_ards
extern main

global _start
_start:
    call init_ards

    mov esp, KERNEL_STACK_TOP
    call main

halt:
    nop;
    jmp halt
