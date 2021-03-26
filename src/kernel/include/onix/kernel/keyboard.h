#ifndef ONIX_KEYBOARD_H
#define ONIX_KEYBOARD_H

#define KEYBOARD_BUFFER_PORT 0X60  // 键盘缓冲区寄存器 端口号为 0x60
#define KEYBOARD_CONTROL_PORT 0X64 // 键盘状态控制寄存器

#define KEY_ESC '\x1b'
#define KEY_BACKSPACE '\b'
#define KEY_TAB '\t'
#define KEY_ENTER '\r'
#define KEY_DELETE '\x7f'

#define CHR_INVISIBLE 0
#define KEY_CTRL_L CHR_INVISIBLE
#define KEY_CTRL_R CHR_INVISIBLE
#define KEY_SHIFT_L CHR_INVISIBLE
#define KEY_SHIFT_R CHR_INVISIBLE
#define KEY_ALT_L CHR_INVISIBLE
#define KEY_ALT_R CHR_INVISIBLE
#define KEY_CAPSLOCK CHR_INVISIBLE

#define CODE_SHFIT_L_DOWN 0x2a
#define CODE_SHFIT_R_DOWN 0x36
#define CODE_ALT_L_DOWN 0x38
#define CODE_ALT_R_DOWN 0xE038
#define CODE_ALT_R_UP 0xE038
#define CODE_CTRL_L_DOWN 0x1D
#define CODE_CTRL_R_DOWN 0xE01D
#define CODE_CTRL_R_UP 0xE09D
#define CODE_CAPSLOCK_DOWN 0x3a

extern bool ctrl_status;
extern bool shift_status;
extern bool alt_status;
extern bool capslock_status;
extern bool ext_status;

void init_keyboard();

#endif