[bits 32]

global get_interrupt
get_interrupt:
    pushf
    cli
    pop eax
    mov edx, eax
    and eax, 0x00000200
    shr eax, 9
    push edx
    popf
    ret

global set_interrupt
set_interrupt:
    mov ecx, [esp + 4]; status
    pushf
    cli
    pop eax
    mov edx, eax

    and eax, 0x00000200
    shr eax, 9

    cmp ecx, 0
    jnz .set
        mov ecx, 0x00000200
        not ecx
        and edx, ecx
        jmp .done
    .set:
        mov ecx, 0x00000200
        or edx, ecx
    .done:
    push edx
    popf
    ret

global current_thread
current_thread:
    mov eax, esp
    and eax, 0xfffff000
    ret
