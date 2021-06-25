%include "boot.inc"

section boot vstart=BOOT_BASE_ADDR

    mov ax, 3
    int 0x10; clean screen

    mov ax, 0
    mov es, ax
    mov ds, ax
    mov ss, ax
    mov sp, BOOT_BASE_ADDR

    mov si, message
    call bios_print

    call load_loader

    jmp 0:LOADER_BASE_ADDR

    ud2; should never here

load_loader:
    mov ecx, LOADER_SECTOR_START
    mov edi, LOADER_BASE_ADDR
    mov bl, LOADER_SECTOR_SIZE ; 2KB
    call read_disk
    ret

BIOS_PRINT_FUNCTION
READ_DISK_FUNCTION

message:
    db "Onix is booting....", 10, 13, 0; 10 表示换行，13 表示光标移到开头，0 表示字符串结束

times 510-($-$$) db 0
db 0x55, 0xaa ; 最后两个字节必须是 0x55 0xaa