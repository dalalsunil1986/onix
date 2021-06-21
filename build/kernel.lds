SECTIONS
{
    . = SIZEOF_HEADERS;
    .text ALIGN(0x1000): {
        *(.text);
    }
    .rodata ALIGN(0x1000): {
        *(.rodata);
    }
    .eh_frame ALIGN(0x1000): {
        *(.eh_frame);
    }
    .data ALIGN(0x1000): {
        *(.data);
    }
    .bss ALIGN(0x1000): {
        *(.bss);
    }

    /DISCARD/ : {
        *(.comment);
    }
}