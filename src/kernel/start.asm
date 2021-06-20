%include "boot.inc"

[bits 32]
section .text

global _start
_start:
    mov byte [gs:0], 'K'

    BMB;
    jmp $
