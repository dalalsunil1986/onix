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
LOADER_STACK_TOP equ LOADER_BASE_ADDR

; 将 kernel 加载到 KERNEL_LOAD_ADDR 的位置
KERNEL_LOAD_ADDR    equ 0xC0010000 ; kernel.bin 加载的物理内存地址
KERNEL_BASE_ADDR    equ 0xC0011000 ; kernel 的入口地址
KERNEL_STACK_TOP    equ 0xC0010000 ; kernel 初始的栈顶
KERNEL_SECTOR_START equ 10  ; kernel.bin 在硬盘上的起始扇区
KERNEL_SECTOR_SIZE  equ 200 ; kernel.bin 在硬盘上占用几个扇区 100KB

PAGE_DIR_ADDR   equ 0x2000
PAGE_SIZE       equ 0x1000 ; 4K = 4096
PAGE_ATTR       equ 0b111
BASE_ADDRESS_LIMIT  equ 0x100000; 1M 的空间

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
    ; 读取硬盘
    ; edi - 存储内存位置
    ; ecx - 存储起始的扇区位置
    ; bl - 存储扇区数量
    pushad; ax, cx, dx, bx, sp, bp, si, di
    ; pushad; ax, cx, dx, bx, sp, bp, si, di

    mov dx, 0x1f1
    mov al, 0
    out dx, al;

    mov dx, 0x1f2
    mov al, bl
    out dx, al; 设置扇区数量

    mov al, cl
    inc dx; 0x1f3
    out dx, al; 起始扇区位置低八位

    shr ecx, 8
    mov al, cl
    inc dx; 0x1f4
    out dx, al; 起始扇区位置中八位

    shr ecx, 8
    mov al, cl
    inc dx; 0x1f5
    out dx, al; 起始扇区位置高八位

    shr ecx, 8
    and cl, 0b1111

    inc dx; 0x1f6
    mov al, 0b1110_0000
    or al, cl
    out dx, al

    inc dx; 0x1f7
    mov al, 0x20; 读硬盘
    out dx, al

    xor ecx, ecx
    mov cl, bl

    .read:
        push cx
        call .waits
        call .reads
        pop cx
        loop .read

    popad
    ; popa
    ret

.waits:
    mov dx, 0x1f7
    .check:
        nop
        nop
        nop ; 一点延迟

        in al, dx
        and al, 0b1000_1000
        cmp al, 0b0000_1000
        jnz .check
    ret

.reads:
    mov dx, 0x1f0
    mov cx, 256
    .readw:
        nop
        nop
        nop

        in ax, dx
        mov [edi], ax
        add edi, 2

        loop .readw
    ret
%endif
%endmacro

%macro BMB 0
xchg bx, bx; 用于 bochs 中断
%endmacro
