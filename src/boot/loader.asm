%include "boot.inc"

section loader vstart=LOADER_BASE_ADDR

    mov si, message
    call bios_print

check_memory:
    ; 以下开始检测内存
    xor ebx, ebx
    mov edx, 0x534d4150; swap
    mov di, ards_buffer

.next:
    mov eax, 0xe820
    mov ecx, 20
    int 0x15

    jc .error

    add di, cx
    inc word [ards_count]
    cmp ebx, 0
    jnz .next
    ; 检测内存成功，跳转到准备保护模式
    jmp prepare_protect_mode

.error:
    ; 检测内存失败，直接挂起
    mov si, message_memory_error
    call bios_print
    jmp halt

halt:
    jmp $

BIOS_PRINT_FUNCTION

prepare_protect_mode:
    cli; 关中断

    lgdt [gdt_ptr]

    in al, 0x92
    or al, 0b0000_0010
    out 0x92, al; 打开 a20 

    mov eax, cr0
    or eax, 1
    mov cr0, eax; 设置 Protection Enable

    jmp word code_selector:protect_mode_start

    ud2; 理论上，不可能到达这里

[bits 32]
; 一下为 32 为模式程序
protect_mode_start:
    mov ax, data_selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov ss, ax
    mov esp, LOADER_STACK_TOP

    mov ax, video_selector
    mov gs, ax

    call load_kernel

    xor ebx, ebx
    mov bx, [ards_count]

    xor ecx, ecx
    mov ecx, ards_buffer

    jmp code_selector: KERNEL_BASE_ADDR
    ud2 ; 显然不可能到这里

load_kernel:
    mov eax, KERNEL_SECTOR_START
    mov ebx, KERNEL_LOAD_ADDR
    mov ecx, KERNEL_SECTOR_SIZE
    call read_disk
    ret

READ_DISK_FUNCTION

; ------------------------------------------------

message db "Loader is starting...", 13, 10, 0
message_memory_error db "Onix Loader Check Memory Error!!!", 13, 10, 0

; ------------------------------------------------

ards_count:
    dw 0
ards_buffer:
    times 20 * 10 db 0

; ------------------------------------------------

code_selector equ (1 << 3)
data_selector equ (2 << 3)
video_selector equ (3 << 3)

code_base equ 0
code_limit equ (0x100000 - 1)

data_base equ 0
data_limit equ (0x100000 - 1)

video_base equ 0xb8000
video_limit equ (0xBFFFF - 0xB8000)

gdt_ptr:
    dw (gdt_end - gdt_base - 1); limit
    dw gdt_base

gdt_base:
    ; 第零号描述符
    dd 0, 0

gdt_code:
    ; 第一号描述符 - 代码段
    dw code_limit & 0xffff; 段界限 0 ~ 15
    dw code_base & 0xffff; 段基地址 0 ~ 15
    db (code_base >> 16) & 0xff; 段基地址 16 ~ 23
    db 0b_1_00_1_1010; attribute 
    db ((code_limit >> 16) & 0xf) | 0b1_1_0_0_0000 ; 段界限 16~ 19 / attribute 2
    db (code_base >> 24) & 0xff; 段基地址 31 ~ 24

gdt_data:
    ; 第二号描述符 - 数据段
    dw data_limit & 0xffff; 段界限 0 ~ 15
    dw data_base & 0xffff; 段基地址 0 ~ 15
    db (data_base >> 16) & 0xff; 段基地址 16 ~ 23
    db 0b_1_00_1_0010; attribute 
    db ((data_limit >> 16) & 0xf) | 0b1_1_0_0_0000 ; 段界限 16~ 19 / attribute 2
    db (data_base >> 24) & 0xff; 段基地址 31 ~ 24

gdt_video:
    ; 第三号描述符 - video
    dw video_limit & 0xffff; 段界限 0 ~ 15
    dw video_base & 0xffff; 段基地址 0 ~ 15
    db (video_base >> 16) & 0xff; 段基地址 16 ~ 23
    db 0b_1_00_1_0010; attribute 
    db ((video_limit >> 16) & 0xf) | 0b0_1_0_0_0000 ; 段界限 16~ 19 / attribute 2
    db (video_base >> 24) & 0xff; 段基地址 31 ~ 24
gdt_end:
