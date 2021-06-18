; (C) Copyright 2021 Steven;
; @author: Steven kangweibaby@163.com
; @date: 2021-04-16

; 在机器肯定有的 1M 物理内存中
; 0x500 ~ 0x7BFF 有 29.75 KB 的可用区域
; 0x7E00 ~ 0x9FBFF 有 607.6KB 的可用区域
; 由于物理内存大小事先不知道，但是以上两个区域应该肯定可以使用;

BOOT_BASE_ADDR equ 0x7c00 ; 主引导扇区加载的位置

; 将 loader 加载到 LOADER_BASE_ADDR 的位置，这里的设置一定要与 makefile 中的一致
LOADER_BASE_ADDR equ 0x1000 ; loader.bin 加载的物理内存地址
LOADER_SECTOR_START equ 0x2 ; loader.bin 在硬盘上的起始扇区
LOADER_SECTOR_SIZE  equ 4 ; loader.bin 在硬盘上占用几个扇区


; 将 kernel 加载到 KERNEL_LOAD_ADDR 的位置
KERNEL_LOAD_ADDR    equ 0x20000 ; kernel.bin 加载的物理内存地址
KERNEL_BASE_ADDR    equ 0x21000 ; kernel 的入口地址
KERNEL_STACK_TOP    equ 0x20000 ; kernel 初始的栈顶
KERNEL_SECTOR_START equ 100 ; kernel.bin 在硬盘上的起始扇区
KERNEL_SECTOR_SIZE  equ 200 ; kernel.bin 在硬盘上占用几个扇区 100KB

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
    ; 从硬盘读取若干逻辑扇区
    ; EAX = 逻辑扇区号
    ; DS: EBX = 目标缓冲区地址
    ; CL, 读取的扇区数量
    pusha

    push eax

    ; 设置需要读取的扇区数
    mov dx, 0x1f2
    mov al, cl
    out dx, al

    pop eax

    ; 写入 LBA地址 0 ~ 7
    inc dx  ;0x1f3
    out dx, al;

    ; 写入 LBA地址 8 ~ 15
    inc dx ;0x1f4
    shr eax, 8
    out dx, al

    ; 写入 LBA地址 16 ~ 23
    inc dx ;0x1f5
    shr eax, 8
    out dx, al

    ; 写入 第一硬盘  LBA地址27~24
    inc dx ;0x1f6
    shr eax, 8
    and al, 0b0000_1111
    or al, 0b1110_0000
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

        xor ax, ax
        mov al, cl  ; 读取多少个扇区
        mov dx, 256 ; 每个扇区共要读取的字数
        mul dx ; ax = cl * 256
        mov cx, ax
        mov dx, 0x1f0 ; 读取端口号
        ; xchg bx, bx
    .readw:
        in ax,dx ; 读取输入
        mov [ebx], ax ; 转移到内存
        add ebx , 2; 一次读取两个字节
        loop .readw
        ; xchg bx,bx 
    popa
    ret

%endif
%endmacro