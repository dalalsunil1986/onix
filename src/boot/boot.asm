%include 'boot.inc'

section boot vstart=BOOT_BASE_ADDR

    ;clean screen
    BIOS_CLEAR_SCREEN

    ;setup segment register
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov sp, BOOT_BASE_ADDR

    ;print message
    mov si, message
    call bios_print

finish:
    sti ; open interrupt
    hlt ; halt cpu
    jmp finish

BIOS_PRINT_FUNCTION

    message db "Hello Onix", 13, 10, 0
    times 510-($-$$) db 0
    db 0x55, 0xaa