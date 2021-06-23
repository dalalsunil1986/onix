; (C) Copyright 2021 Steven;
; @author: Steven kangweibaby@163.com
; @date: 2021-06-23

[bits 32]

section .text

global thread_switch
thread_switch:
    mov eax, esp;
    and eax, 0xfffff000; current

    mov ecx, [esp + 4]; thread

    push ebx
    push ebp
    push esi
    push edi
    mov [eax], esp

    ; 以上保存当前线程信息
    ; xchg bx, bx
    ; 以下恢复目标线程信息

    mov esp, [ecx]
    pop edi
    pop esi
    pop ebp
    pop ebx

    ret
