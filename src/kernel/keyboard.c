#include <onix/kernel/interrupt.h>
#include <onix/kernel/keyboard.h>
#include <onix/kernel/io.h>
#include <onix/kernel/debug.h>

void keyboard_handler(int vector)
{
    u8 chr = inb(KEYBOARD_BUFFER_PORT);
    DEBUGK("key %c %d\n", chr, chr);
}

void init_keyboard()
{
    handler_table[ICW2_INT_VECTOR_IRQ0 + IRQ_KEYBOARD] = keyboard_handler;
}