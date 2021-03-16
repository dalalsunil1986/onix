%ifndef ONIX_BOOT_INC
%define ONIX_BOOT_INC

BOOT_BASE_ADDR      equ 0x7c00

LOADER_BASE_ADDR    equ 0x900
LOADER_START_SECTOR equ 0x2

%macro BIOS_CLEAR_SCREEN 0
    mov ax, 0x0003
    int 0x10
%endmacro

%macro BIOS_PRINT_FUNCTION 0
%ifndef BIOS_PRINT_FUNCTION
%define BIOS_PRINT_FUNCTION
bios_print:
    cld
.bios_print_loop:
    lodsb
    or al, al
    jz .bios_print_done
    mov ah, 0x0E
    int 0x10
    jmp .bios_print_loop
.bios_print_done:
    ret
%endif
%endmacro

%macro READ_DISK_FUNCTION 0
%ifndef READ_DISK_FUNCTION
%define READ_DISK_FUNCTION
read_disk: 
    ;从硬盘读取一个逻辑扇区
    ; EAX = 逻辑扇区号
    ; DS: EBX = 目标缓冲区地址
    ; CX, 读取的扇区数量
        pusha

        push eax

        ; 设置需要读取的扇区数
        mov dx, 0x1f2
        mov al, cl
        out dx, al

        pop eax

        ; 写入 LBA地址 7 ~ 0
        inc dx  ;0x1f3
        out dx, al;

        ; 写入 LBA地址 15 ~ 8
        inc dx ;0x1f4
        shr eax, 8
        out dx, al

        ; 写入 LBA地址 23 ~ 16
        inc dx ;0x1f5
        shr eax, 8
        out dx, al

        ; 写入 第一硬盘  LBA地址27~24
        inc dx ;0x1f6
        shr eax, 8
        and al, 0x0f
        or al, 0xe0
        out dx,al

        ; 写入读命令
        inc dx ;0x1f7
        mov al, 0x20
        out dx,al

    .waits:
        in al,dx
        and al,0x88
        cmp al,0x08
        jnz .waits ; 不忙，且硬盘已准备好数据传输 

        mov ax, cx
        mov dx, 256 ;总共要读取的字数
        mul dx
        mov cx, ax
        mov dx, 0x1f0 ; 读取端口号

    .readw:
        in ax,dx ; 读取输入
        mov [ebx], ax ; 转移到内存
        add ebx , 2; 一次读取两个字节

        loop .readw

        popa

        ret
%endif
%endmacro

%endif