%include "boot.inc"
%include "elf.inc"

LOADER_STACK_TOP equ LOADER_BASE_ADDR

section loader vstart=LOADER_BASE_ADDR
    mov si, message
    call bios_print ; 打印字符串

halt:
    jmp $

BIOS_PRINT_FUNCTION

message db "Hello, Loader!!!", 13, 10, 0

READ_DISK_FUNCTION
