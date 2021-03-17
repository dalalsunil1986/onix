%include "boot.inc"

bits 32
[section .text]

global _start
global halt

extern main
extern init_kernel
extern gdt_ptr

_start:
    sgdt [gdt_ptr]
    call init_kernel
    lgdt [gdt_ptr]
    call main

halt:
    ; sti
    ; hlt
    xchg bx, bx
    jmp halt
