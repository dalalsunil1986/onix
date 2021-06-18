; (C) Copyright 2021 Steven;
; @author: Steven kangweibaby@163.com
; @date: 2021-06-18

%include "boot.inc"

section boot vstart=BOOT_BASE_ADDR

    mov ax, 3 ; ah = 0, al = 3 设置屏幕模式为 80 * 25 文本模式
    int 0x10 ; 调用 BIOS 中断清除屏幕

    mov ax, 0
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, BOOT_BASE_ADDR ; 将栈顶指针指向 0x7c00

    mov si, message
    call bios_print ; 打印字符串

    call load_loader ; 从硬盘读取 loader.bin 到指定位置

    jmp 0:LOADER_BASE_ADDR; 跳转到 loader.bin 中继续执行

halt:
    jmp $

load_loader:
    mov eax, LOADER_SECTOR_START
    mov bx, LOADER_BASE_ADDR
    mov cl, LOADER_SECTOR_SIZE
    call read_disk
    ret

READ_DISK_FUNCTION
BIOS_PRINT_FUNCTION

message:
    db "Onix is booting....", 10, 13, 0
    ;13 表示将光标移到开头，10 表示将光标移到下一行

times 510-($-$$) db 0
db 0x55, 0xaa ; 最后两个字节必须是 0x55 0xaa