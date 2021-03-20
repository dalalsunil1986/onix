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

    ;print loading loader
    mov si, message_load_loader
    call bios_print

    call load_loader
    ; xchg bx, bx
    jmp 0:LOADER_BASE_ADDR

finish:
    sti ; open interrupt
    hlt ; halt cpu
    jmp finish

load_loader:
    mov eax, LOADER_START_SECTOR
    mov bx, LOADER_BASE_ADDR
    mov cx, LOADER_SECTOR_SIZE ; 2KB
    call read_disk

READ_DISK_FUNCTION

BIOS_PRINT_FUNCTION

    message_load_loader db "Loading Onix loader...", 13, 10, 0
    times 510-($-$$) db 0
    db 0x55, 0xaa