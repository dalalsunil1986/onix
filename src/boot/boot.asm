%include "boot.inc"

section boot vstart=BOOT_BASE_ADDR

    mov ax, 3
    int 0x10; clean screen

    mov si, message
    call bios_print

    call load_loader
    jmp 0:LOADER_BASE_ADDR

    ud2; should never here

load_loader:
    mov eax, LOADER_START_SECTOR
    mov bx, LOADER_BASE_ADDR
    mov cx, LOADER_SECTOR_SIZE ; 2KB
    call read_disk

BIOS_PRINT_FUNCTION
READ_DISK_FUNCTION

message:
    db "Onix is booting....", 10, 13, 0; 10 表示换行，13 表示光标移到开头，0 表示字符串结束

times 510-($-$$) db 0
db 0x55, 0xaa ; 最后两个字节必须是 0x55 0xaa