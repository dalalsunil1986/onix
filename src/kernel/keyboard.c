#include <onix/kernel/interrupt.h>
#include <onix/kernel/keyboard.h>
#include <onix/kernel/io.h>
#include <onix/kernel/debug.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

bool ctrl_status;
bool shift_status;
bool alt_status;
bool capslock_status;
bool ext_status;

static char keymap[][2] = {
    /* 扫描码   未与shift组合  与shift组合*/
    /* ---------------------------------- */
    /* 0x00 */ {0, 0},
    /* 0x01 */ {KEY_ESC, KEY_ESC},
    /* 0x02 */ {'1', '!'},
    /* 0x03 */ {'2', '@'},
    /* 0x04 */ {'3', '#'},
    /* 0x05 */ {'4', '$'},
    /* 0x06 */ {'5', '%'},
    /* 0x07 */ {'6', '^'},
    /* 0x08 */ {'7', '&'},
    /* 0x09 */ {'8', '*'},
    /* 0x0A */ {'9', '('},
    /* 0x0B */ {'0', ')'},
    /* 0x0C */ {'-', '_'},
    /* 0x0D */ {'=', '+'},
    /* 0x0E */ {KEY_BACKSPACE, KEY_BACKSPACE},
    /* 0x0F */ {KEY_TAB, KEY_TAB},
    /* 0x10 */ {'q', 'Q'},
    /* 0x11 */ {'w', 'W'},
    /* 0x12 */ {'e', 'E'},
    /* 0x13 */ {'r', 'R'},
    /* 0x14 */ {'t', 'T'},
    /* 0x15 */ {'y', 'Y'},
    /* 0x16 */ {'u', 'U'},
    /* 0x17 */ {'i', 'I'},
    /* 0x18 */ {'o', 'O'},
    /* 0x19 */ {'p', 'P'},
    /* 0x1A */ {'[', '{'},
    /* 0x1B */ {']', '}'},
    /* 0x1C */ {KEY_ENTER, KEY_ENTER},
    /* 0x1D */ {KEY_CTRL_L, KEY_CTRL_L},
    /* 0x1E */ {'a', 'A'},
    /* 0x1F */ {'s', 'S'},
    /* 0x20 */ {'d', 'D'},
    /* 0x21 */ {'f', 'F'},
    /* 0x22 */ {'g', 'G'},
    /* 0x23 */ {'h', 'H'},
    /* 0x24 */ {'j', 'J'},
    /* 0x25 */ {'k', 'K'},
    /* 0x26 */ {'l', 'L'},
    /* 0x27 */ {';', ':'},
    /* 0x28 */ {'\'', '"'},
    /* 0x29 */ {'`', '~'},
    /* 0x2A */ {KEY_SHIFT_L, KEY_SHIFT_L},
    /* 0x2B */ {'\\', '|'},
    /* 0x2C */ {'z', 'Z'},
    /* 0x2D */ {'x', 'X'},
    /* 0x2E */ {'c', 'C'},
    /* 0x2F */ {'v', 'V'},
    /* 0x30 */ {'b', 'B'},
    /* 0x31 */ {'n', 'N'},
    /* 0x32 */ {'m', 'M'},
    /* 0x33 */ {',', '<'},
    /* 0x34 */ {'.', '>'},
    /* 0x35 */ {'/', '?'},
    /* 0x36	*/ {KEY_SHIFT_R, KEY_SHIFT_R},
    /* 0x37 */ {'*', '*'},
    /* 0x38 */ {KEY_ALT_L, KEY_ALT_L},
    /* 0x39 */ {' ', ' '},
    /* 0x3A */ {KEY_CAPSLOCK, KEY_CAPSLOCK}};

void keyboard_handler(int vector)
{
    bool last_shift = shift_status;
    bool last_capslock = capslock_status;

    u16 scancode = inb(KEYBOARD_BUFFER_PORT);
    if (scancode == 0xe0)
    {
        ext_status = true;
        return;
    }
    if (ext_status)
    {
        scancode = scancode | 0xe000;
        ext_status = false;
    }

    bool break_code = ((scancode & 0x0080) != 0);
    if (break_code)
    {
        u16 makecode = (scancode &= 0xff7f);

        if (makecode == CODE_CTRL_L_DOWN || makecode == CODE_CTRL_R_DOWN)
        {
            ctrl_status = false;
        }
        else if (makecode == CODE_SHFIT_L_DOWN || makecode == CODE_SHFIT_R_DOWN)
        {
            shift_status = false;
        }
        else if (makecode == CODE_ALT_L_DOWN || makecode == CODE_ALT_R_DOWN)
        {
            alt_status = false;
        }
        return;
    }

    if (scancode == CODE_CTRL_L_DOWN || scancode == CODE_CTRL_R_DOWN)
    {
        ctrl_status = true;
        return;
    }

    if (scancode == CODE_SHFIT_L_DOWN || scancode == CODE_SHFIT_R_DOWN)
    {
        shift_status = true;
        return;
    }

    if (scancode == CODE_ALT_L_DOWN || scancode == CODE_ALT_R_DOWN)
    {
        alt_status = true;
        return;
    }

    if (scancode == CODE_CAPSLOCK_DOWN)
    {
        capslock_status = !capslock_status;
        return;
    }

    bool shift = false;
    if (last_shift && last_capslock)
    {
        shift = false;
    }
    else if (last_shift || last_capslock)
    {
        shift = true;
    }

    if (
        (scancode > 0 && scancode < 0x3b) ||
        (scancode == CODE_ALT_R_DOWN) ||
        (scancode == CODE_CTRL_R_DOWN))
    {
        u8 index = (scancode &= 0x00ff);
        char ch = keymap[index][shift];
        if (ch)
        {
            DEBUGP("%c", ch);
        }
    }

    // DEBUGK("key %c %d\n", chr, chr);
}

void init_keyboard()
{
    register_handler(IRQ_KEYBOARD, keyboard_handler);
}