section boot vstart=0x7c00

    ;clean screen
    mov ax, 3
    int 0x10;

    ;setup segment register
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov sp, 0x7c00

    ;print message
    mov si, message
    call print

finish:
    sti ; open interrupt
    hlt ; halt cpu
    jmp finish

print:
    cld
    .print_loop:
        lodsb
        or al, al
        jz .print_done
        mov ah, 0x0e;
        int 0x10;
        jmp .print_loop
    .print_done:
        ret


    message db "Hello Onix", 13, 10, 0
    times 510-($-$$) db 0
    db 0x55, 0xaa