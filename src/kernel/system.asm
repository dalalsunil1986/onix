%include "boot.inc"

bits 32
[section .text]

global io_outb
global io_inb

io_outb:
    mov edx, [esp + 4];  port
    mov al, [esp + 8]; byte
    out dx, al
    nop;
    nop; a little delay
    ret

io_inb:
    mov edx, [esp + 4]; port
    xor eax, eax
    in al, dx
    nop;
    nop; a little delay
    ret
