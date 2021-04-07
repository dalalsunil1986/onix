%include "boot.inc"
; interrupt.........
bits 32
section .text

%define ERRORCODE nop
%define PLACEHOLDER push 0

extern handler_table

section .data
global interrupt_entry_table
interrupt_entry_table:

%macro INTERRUPT_HANDLER 2
section .text
interrupt_%1:
    ; xchg bx, bx
    %2
    push ds
    push es
    push fs
    push gs
    pushad

    mov al, 0x20 ; eoi command
    out 0xa0, al ; slave pic chip
    out 0x20, al ; master pic chip

    push %1
    call [handler_table + %1 * 4]
    jmp __interrupt_exit; 所有的中断处理程序共用一个中断退出函数；

section .data
    dd interrupt_%1
%endmacro

section .text
global interrupt_exit
interrupt_exit:
    ; BMB;
    mov eax, [esp + 4]; stack top
    mov esp, eax
    ; xchg bx, bx
global __interrupt_exit
__interrupt_exit:
    ; BMB;
    add esp, 4
    popad
    pop gs
    pop fs
    pop es
    pop ds
    add esp, 4
    iretd


section .text

INTERRUPT_HANDLER 0x00,PLACEHOLDER
INTERRUPT_HANDLER 0x01,PLACEHOLDER
INTERRUPT_HANDLER 0x02,PLACEHOLDER
INTERRUPT_HANDLER 0x03,PLACEHOLDER 
INTERRUPT_HANDLER 0x04,PLACEHOLDER
INTERRUPT_HANDLER 0x05,PLACEHOLDER
INTERRUPT_HANDLER 0x06,PLACEHOLDER
INTERRUPT_HANDLER 0x07,PLACEHOLDER 
INTERRUPT_HANDLER 0x08,ERRORCODE
INTERRUPT_HANDLER 0x09,PLACEHOLDER
INTERRUPT_HANDLER 0x0a,ERRORCODE
INTERRUPT_HANDLER 0x0b,ERRORCODE 
INTERRUPT_HANDLER 0x0c,PLACEHOLDER
INTERRUPT_HANDLER 0x0d,ERRORCODE
INTERRUPT_HANDLER 0x0e,ERRORCODE
INTERRUPT_HANDLER 0x0f,PLACEHOLDER 
INTERRUPT_HANDLER 0x10,PLACEHOLDER
INTERRUPT_HANDLER 0x11,ERRORCODE
INTERRUPT_HANDLER 0x12,PLACEHOLDER
INTERRUPT_HANDLER 0x13,PLACEHOLDER 
INTERRUPT_HANDLER 0x14,PLACEHOLDER
INTERRUPT_HANDLER 0x15,PLACEHOLDER
INTERRUPT_HANDLER 0x16,PLACEHOLDER
INTERRUPT_HANDLER 0x17,PLACEHOLDER 
INTERRUPT_HANDLER 0x18,ERRORCODE
INTERRUPT_HANDLER 0x19,PLACEHOLDER
INTERRUPT_HANDLER 0x1a,ERRORCODE
INTERRUPT_HANDLER 0x1b,ERRORCODE 
INTERRUPT_HANDLER 0x1c,PLACEHOLDER
INTERRUPT_HANDLER 0x1d,ERRORCODE
INTERRUPT_HANDLER 0x1e,ERRORCODE
INTERRUPT_HANDLER 0x1f,PLACEHOLDER 

INTERRUPT_HANDLER 0x20,PLACEHOLDER ; clock
INTERRUPT_HANDLER 0x21,PLACEHOLDER ; keyboard
INTERRUPT_HANDLER 0x22,PLACEHOLDER ;
INTERRUPT_HANDLER 0x23,PLACEHOLDER ;
INTERRUPT_HANDLER 0x24,PLACEHOLDER ;
INTERRUPT_HANDLER 0x25,PLACEHOLDER ;
INTERRUPT_HANDLER 0x26,PLACEHOLDER ;
INTERRUPT_HANDLER 0x27,PLACEHOLDER ;
INTERRUPT_HANDLER 0x28,PLACEHOLDER ;
INTERRUPT_HANDLER 0x29,PLACEHOLDER ;
INTERRUPT_HANDLER 0x2A,PLACEHOLDER ;
INTERRUPT_HANDLER 0x2B,PLACEHOLDER ;
INTERRUPT_HANDLER 0x2C,PLACEHOLDER ;
INTERRUPT_HANDLER 0x2D,PLACEHOLDER ;
INTERRUPT_HANDLER 0x2E,PLACEHOLDER ;
INTERRUPT_HANDLER 0x2F,PLACEHOLDER ;


[bits 32]
extern syscall_table
section .text
global syscall_handler
syscall_handler:
   push 0 ; 压入0, 使栈中格式统一

   push ds
   push es
   push fs
   push gs
   pushad ; PUSHAD指令压入32位寄存器，其入栈顺序是:
            ; EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI 

   push 0x80 ; 此位置压入0x80也是为了保持统一的栈格式

    ;2 为系统调用子功能传入参数
   push edx ; 系统调用中第3个参数
   push ecx ; 系统调用中第2个参数
   push ebx ; 系统调用中第1个参数

    ;3 调用子功能处理函数
   call [syscall_table + eax*4] ; 编译器会在栈中根据C函数声明匹配正确数量的参数
   add esp, 12; 跨过上面的三个参数

    ;4 将call调用后的返回值存入待当前内核栈中eax的位置
   mov [esp + 8*4], eax
   jmp __interrupt_exit; intr_exit返回,恢复上下文
