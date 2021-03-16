%include "boot.inc"

LOADER_STACK_TOP equ LOADER_BASE_ADDR

section loader vstart=LOADER_BASE_ADDR

    jmp start

gdt_base:   GDTDescriptor 0,        0, 0
gdt_code:   GDTDescriptor 0,        0xfffff, GDT_CODE_ATTRIBUTE
gdt_data:   GDTDescriptor 0,        0xfffff, GDT_DATA_ATTRIBUTE
gdt_video:  GDTDescriptor 0xb8000,  0x0ffff, GDT_VIDEO_ATTRIBUTE
gdt_size:   equ ($ - gdt_base)
gdt_limit:  equ (gdt_size - 1)
gdt_ptr:    dw gdt_limit
            dd gdt_base

SELECTOR_CODE   equ (1 << 3) | TI_GDT | RPL0
SELECTOR_DATA   equ (2 << 3) | TI_GDT | RPL0
SELECTOR_VIDEO  equ (3 << 3) | TI_GDT | RPL0

total_memory_bytes dd 0
ards_count  dw 0
ards_buffer times 128 - ($ - $$) db 0

start:
    ; xchg bx, bx

    mov si, message_start_loader
    call bios_print

    ; xchg bx, bx
    ; detect memory
    xor ebx, ebx
    mov edx, 0x534d4150
    mov di, ards_buffer

    mov si, message_detect_momory_e820
    call bios_print

    ; 内存检测可能会修改内存内容，待验证

.check_memory_r820:
    mov eax, 0x0000e820
    mov ecx, 20
    int 0x15

    jc .get_memory_e801

    add di, cx
    inc word [ards_count]
    cmp ebx, 0
    jnz .check_memory_r820

    mov cx, [ards_count]
    mov ebx, ards_buffer
    xor edx, edx

.get_max_memory_area:

    mov eax, [ebx]
    add eax, [ebx + 8]
    add ebx, 20
    cmp edx, eax
    jge .next_ards
    mov edx, eax

.next_ards:
    loop .get_max_memory_area
    jmp .get_memory_ok

.get_memory_e801:
    mov si, message_detect_momory_e801
    call bios_print

    mov ax, 0xe801
    mul cx
    shl edx, 16
    and eax, 0x0000ffff
    or edx, eax
    add edx, 0x100000
    mov esi, edx

    xor eax, eax
    mov ax, bx
    mov ecx, 0x10000
    mul ecx
    add esi, eax
    mov edx, esi
    jmp .get_memory_ok

.get_memory_e88:
    mov si, message_detect_momory_e88
    call bios_print

    mov ah, 0x88
    int 0x15
    jc .get_memory_failure

    and eax, 0x0000ffff

    mov cx, 0x400
    mul cx

    shl edx, 16
    or edx, eax
    and edx, 0x100000

.get_memory_ok:
    mov [total_memory_bytes], edx

    ; xchg bx, bx
; prepare for protect mode
    mov si, message_prepare_protect_mode
    call bios_print

    cli

    lgdt [gdt_ptr]

    ; open a20 address line
    in al, 0x92
    or al, 0000_0010b
    out 0x92, al

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; xchg  bx, bx

    jmp dword SELECTOR_CODE:protect_mode_start

.get_memory_failure:
finish:
    sti ; open interrupt
    hlt ; halt cpu
    jmp finish

BIOS_PRINT_FUNCTION

message_start_loader            db "Loader is starting...", 13, 10, 0
message_detect_momory_e820      db "Detecting memory e820...", 13, 10, 0
message_detect_momory_e801      db "Detecting memory e801...", 13, 10, 0
message_detect_momory_e88       db "Detecting memory e88...", 13, 10, 0
message_prepare_protect_mode    db "Preparing protect mode...", 13, 10, 0

[bits 32]

protect_mode_start:
    xchg bx, bx
    mov ax, SELECTOR_DATA
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, LOADER_STACK_TOP
    mov ax, SELECTOR_VIDEO

    xchg bx, bx
    mov gs, ax
    xchg ebx, ebx

    jmp $