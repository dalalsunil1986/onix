%include "boot.inc"

bits 32
section .text

global syscall0:
syscall0:
    mov eax, [esp + 4]
    int 0x80
    ret

global syscall1: 
syscall1:
    mov eax, [esp + 4]
    mov ebx, [esp + 8]
    int 0x80
    ret