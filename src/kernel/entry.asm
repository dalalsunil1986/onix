; (C) Copyright 2021 Steven;
; @author: Steven kangweibaby@163.com
; @date: 2021-06-17
; 内核入口

%include "boot.inc"

[bits 32]
extern main
extern ards_count
extern ards_descriptor
extern gdt_ptr

global _start
_start:
    xchg bx, bx

    pop word [gdt_ptr]

    xor eax, eax
    pop ax
    mov [ards_descriptor], eax

    pop word [ards_count]

    mov esp, KERNEL_STACK_TOP
    call main

halt:
    xchg bx, bx
    jmp halt
