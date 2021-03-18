%include "boot.inc"

bits 32
[section .text]

global _start
global halt

extern main
extern __init_kernel
extern gdt_ptr
extern idt_ptr

_start:
    call __init_kernel
    call main

halt:
    sti
    hlt
    ; xchg bx, bx
    jmp halt
