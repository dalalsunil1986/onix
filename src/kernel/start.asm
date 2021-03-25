%include "boot.inc"

bits 32
[section .text]

global _start
global halt

extern __init_kernel
extern gdt_ptr
extern idt_ptr
extern total_memory_bytes
extern ards_count
extern ards_table
extern memcpy

_start:
    ; setup memory
    pop ebx
    mov eax, [ebx]
    mov [total_memory_bytes], eax

    pop ebx
    mov ecx, [ebx]
    mov [ards_count], ecx

    pop ebx;
    mov eax, 20
    mul ecx

    push eax ; count
    push ebx ; ards_buffer
    push ards_table; ards
    call memcpy 

    mov esp, KERNEL_STACK_TOP
    call __init_kernel
    jmp restart

extern init_kernel_task
extern init_stack_top

restart:
    mov eax, [init_stack_top]
    mov esp, eax
    jmp init_kernel_task

halt:
    sti
    hlt
    ; xchg bx, bx
    jmp halt