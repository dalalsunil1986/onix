%ifndef ONIX_BOOT_INC
%define ONIX_BOOT_INC

BOOT_BASE_ADDR equ 0x7c00

%macro BIOS_CLEAR_SCREEN 0
    mov ax, 0x0003
    int 0x10
%endmacro

%macro BIOS_PRINT_FUNCTION 0
%ifndef BIOS_PRINT_FUNCTION
%define BIOS_PRINT_FUNCTION
bios_print:
    cld
bios_print_loop:
    lodsb
    or al, al
    jz bios_print_done
    mov ah, 0x0E
    int 0x10
    jmp bios_print_loop
bios_print_done:
    ret
%endif
%endmacro

%endif