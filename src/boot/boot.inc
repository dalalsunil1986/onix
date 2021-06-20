
BOOT_BASE_ADDR      equ 0x7c00

LOADER_BASE_ADDR    equ 0x1000
LOADER_START_SECTOR equ 0x2
LOADER_SECTOR_SIZE  equ 4


%macro BIOS_PRINT_FUNCTION 0
%ifndef BIOS_PRINT_FUNCTION
%define BIOS_PRINT_FUNCTION
; 将需要打印的字符串地址存到 si 寄存器中
; 字符串以 0 结束
bios_print:
    cld ; 正向增长
    .loop:
        lodsb
        or al, al
        jz .done ; 如果字符为 0 则结束打印
        mov ah, 0x0E ; 0x10 子功能 将 al 中的字符打印到光标位置
        int 0x10 ; 调用 BIOS 打印字符串
        jmp .loop
    .done:
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
        pushad

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
        ; xchg bx, bx
    .readw:
        in ax,dx ; 读取输入
        mov [ebx], ax ; 转移到内存
        add ebx , 2; 一次读取两个字节

        loop .readw
        ; xchg bx,bx 
        popad

        ret
%endif
%endmacro

%macro BMB 0
xchg bx, bx; 用于 bochs 中断
%endmacro
