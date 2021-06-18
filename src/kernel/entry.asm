; (C) Copyright 2021 Steven;
; @author: Steven kangweibaby@163.com
; @date: 2021-06-17
; 内核入口
%include "boot.inc"

[bits 32]
extern main

global _start
_start:
    mov esp, KERNEL_STACK_TOP
    call main
    xchg bx, bx
    xchg bx, bx

halt:
    jmp $

