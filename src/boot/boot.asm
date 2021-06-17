%include "boot.inc"

[org 0x7c00]

xchg bx, bx
mov ax, 3
int 0x10

mov si, message
call bios_print

jmp $

BIOS_PRINT_FUNCTION

message:
    db "hello, booting....", 10, 13, 0; 10 表示换行，13 表示光标移到开头，0 表示字符串结束

times 510-($-$$) db 0
db 0x55, 0xaa ; 最后两个字节必须是 0x55 0xaa