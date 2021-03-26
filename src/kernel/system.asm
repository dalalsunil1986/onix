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
    pushf
    cli
    pop eax
    and eax, 0x00000200
    shr eax, 9
    sti
    ret

global disable_int
disable_int:
    pushf
    cli
    pop eax
    and eax, 0x00000200
    shr eax, 9
    ret

global get_eflags
get_eflags:
    pushf
    pop eax
    ret

global set_eflags
set_eflags:
    push dword [esp + 4]
    popf
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

; memory management

global get_pde:
get_pde:
    mov eax, cr3
    ret

; routine control

global halt
halt:
    sti
    hlt
    jmp halt

global pause
pause:
    sti
    hlt
    cli
    ret