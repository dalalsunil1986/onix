

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
