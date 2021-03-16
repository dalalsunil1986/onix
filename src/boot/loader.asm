%include "boot.inc"

section loader vstart=LOADER_BASE_ADDR

    mov si, message_start_loader
    call bios_print

finish:
    sti ; open interrupt
    hlt ; halt cpu
    jmp finish

BIOS_PRINT_FUNCTION

message_start_loader db "Loader is loaded...", 13, 10, 0
