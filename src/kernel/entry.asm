; (C) Copyright 2021 Steven;
; @author: Steven kangweibaby@163.com
; @date: 2021-06-17
; 内核入口
%include "boot.inc"

[bits 32]
global __kernel_entry
__kernel_entry:
    mov esp, KERNEL_STACK_TOP
    mov byte [gs:0], 'E'
    xchg bx, bx
halt:
    jmp $

