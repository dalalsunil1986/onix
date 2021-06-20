%include "boot.inc"

section loader vstart=LOADER_BASE_ADDR

    mov si, message
    call bios_print

    jmp $


BIOS_PRINT_FUNCTION

message db "Loader is starting...", 13, 10, 0