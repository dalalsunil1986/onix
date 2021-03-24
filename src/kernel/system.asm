%include "boot.inc"

bits 32
[section .text]

global load_gdt
load_gdt:
    mov eax, [esp + 4]
    lgdt [eax]
    ret

global save_gdt
save_gdt:
    mov eax, [esp + 4]
    sgdt [eax]
    ret

global load_idt
load_idt:
    mov eax, [esp + 4]
    lidt [eax]
    ret

global load_tss
load_tss:
    mov eax, [esp + 4]
    ltr [eax]
    ret

global enable_int
enable_int:
    sti
    ret

global disable_int
disable_int:
    cli
    ret

global outb
outb:
    mov edx, [esp + 4];  port
    mov al, [esp + 8]; byte
    out dx, al
    nop;
    nop; a little delay
    ret

global inb
inb:
    mov edx, [esp + 4]; port
    xor eax, eax
    in al, dx
    nop;
    nop; a little delay
    ret

; interrupt.........

%define ERRORCODE nop
%define PLACEHOLDER push 0

extern handler_table

section .data
global interrupt_entry_table
interrupt_entry_table:

%macro INTERRUPT_HANDLER 2
section .text
interrupt_%1:
    %2
    push ds
    push es
    push fs
    push gs
    pushad

    mov al, 0x20 ; eoi command
    out 0xa0, al ; slave pic chip
    out 0x20, al ; master pic chip

    push %1
    call [handler_table + %1 * 4]
    add esp, 4

    popad
    pop gs
    pop fs
    pop es
    pop ds
    add esp, 4
    iretd

section .data
    dd interrupt_%1
%endmacro

section .text

INTERRUPT_HANDLER 0x00,PLACEHOLDER
INTERRUPT_HANDLER 0x01,PLACEHOLDER
INTERRUPT_HANDLER 0x02,PLACEHOLDER
INTERRUPT_HANDLER 0x03,PLACEHOLDER 
INTERRUPT_HANDLER 0x04,PLACEHOLDER
INTERRUPT_HANDLER 0x05,PLACEHOLDER
INTERRUPT_HANDLER 0x06,PLACEHOLDER
INTERRUPT_HANDLER 0x07,PLACEHOLDER 
INTERRUPT_HANDLER 0x08,ERRORCODE
INTERRUPT_HANDLER 0x09,PLACEHOLDER
INTERRUPT_HANDLER 0x0a,ERRORCODE
INTERRUPT_HANDLER 0x0b,ERRORCODE 
INTERRUPT_HANDLER 0x0c,PLACEHOLDER
INTERRUPT_HANDLER 0x0d,ERRORCODE
INTERRUPT_HANDLER 0x0e,ERRORCODE
INTERRUPT_HANDLER 0x0f,PLACEHOLDER 
INTERRUPT_HANDLER 0x10,PLACEHOLDER
INTERRUPT_HANDLER 0x11,ERRORCODE
INTERRUPT_HANDLER 0x12,PLACEHOLDER
INTERRUPT_HANDLER 0x13,PLACEHOLDER 
INTERRUPT_HANDLER 0x14,PLACEHOLDER
INTERRUPT_HANDLER 0x15,PLACEHOLDER
INTERRUPT_HANDLER 0x16,PLACEHOLDER
INTERRUPT_HANDLER 0x17,PLACEHOLDER 
INTERRUPT_HANDLER 0x18,ERRORCODE
INTERRUPT_HANDLER 0x19,PLACEHOLDER
INTERRUPT_HANDLER 0x1a,ERRORCODE
INTERRUPT_HANDLER 0x1b,ERRORCODE 
INTERRUPT_HANDLER 0x1c,PLACEHOLDER
INTERRUPT_HANDLER 0x1d,ERRORCODE
INTERRUPT_HANDLER 0x1e,ERRORCODE
INTERRUPT_HANDLER 0x1f,PLACEHOLDER 

INTERRUPT_HANDLER 0x20,PLACEHOLDER ; clock
INTERRUPT_HANDLER 0x21,PLACEHOLDER ; keyboard
INTERRUPT_HANDLER 0x22,PLACEHOLDER ;
INTERRUPT_HANDLER 0x23,PLACEHOLDER ;
INTERRUPT_HANDLER 0x24,PLACEHOLDER ;
INTERRUPT_HANDLER 0x25,PLACEHOLDER ;
INTERRUPT_HANDLER 0x26,PLACEHOLDER ;
INTERRUPT_HANDLER 0x27,PLACEHOLDER ;
INTERRUPT_HANDLER 0x28,PLACEHOLDER ;
INTERRUPT_HANDLER 0x29,PLACEHOLDER ;
INTERRUPT_HANDLER 0x2A,PLACEHOLDER ;
INTERRUPT_HANDLER 0x2B,PLACEHOLDER ;
INTERRUPT_HANDLER 0x2C,PLACEHOLDER ;
INTERRUPT_HANDLER 0x2D,PLACEHOLDER ;
INTERRUPT_HANDLER 0x2E,PLACEHOLDER ;
INTERRUPT_HANDLER 0x2F,PLACEHOLDER ;



; memory management

global get_pde:
get_pde:
    mov eax, cr3
    ret


; tasks
global switch_to
switch_to:
    ; xchg bx, bx
    mov eax, [esp + 4]; cur
    mov ecx, [esp + 8]; next

    push esi
    push edi
    push ebx
    push ebp

    mov [eax], esp
    push ecx
    push eax

global jump_to_next;
jump_to_next:
    ; xchg bx, bx;
    mov eax, [esp + 4]; next
    mov esp, [eax];

    pop ebp
    pop ebx
    pop edi
    pop esi
    sti
    ret
