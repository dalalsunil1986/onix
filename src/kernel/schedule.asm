; tasks

bits 32
section .text

global switch_to
switch_to:
    xchg bx, bx
    mov eax, [esp + 4]; cur
    mov ecx, [esp + 8]; next

    push esi
    push edi
    push ebx
    push ebp
    mov [eax], esp
    ; 以上保存当前线程的堆栈信息

    ; xchg bx, bx

    ; 以下恢复需要切换到线程的堆栈信息
    mov esp, [ecx];
    pop ebp
    pop ebx
    pop edi
    pop esi
    ret
