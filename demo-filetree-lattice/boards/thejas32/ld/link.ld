/*
 * Reference linker script for the THEJAS32 board.
 *
 * TODO: replace ORIGIN/LENGTH with the real memory map from the SoC manual.
 * The ASSERTs are deliberately conservative; they fail the link rather than
 * letting .bss or the stack overlap.
 */

ENTRY(_start)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 512K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

SECTIONS
{
    .text :
    {
        KEEP(*(.text.init))
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    .data :
    {
        __data_start = .;
        *(.data*)
        . = ALIGN(4);
        __data_end = .;
    } > RAM AT > FLASH
    __data_load_start = LOADADDR(.data);

    .bss (NOLOAD) :
    {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > RAM

    . = ALIGN(8);
    __stack_top = ORIGIN(RAM) + LENGTH(RAM);
    _estack = __stack_top;

    ASSERT(__data_end <= __stack_top - 0x8000, "RAM overflow: data + bss + stack")
    ASSERT(__stack_top - __bss_end >= 0x4000, "less than 16 KiB left for stack/heap")
}
