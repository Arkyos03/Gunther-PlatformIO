/*
 * Gunther.ino - common section layout (Cortex-M4F, GCC / newlib)
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License
 */

ENTRY(Reset_Handler)

STACK_SIZE = 0x4000;   /* 16 KB of stack */

SECTIONS
{
  .text :
  {
    . = ALIGN(4);
    KEEP(*(.vectors))
    *(.text*)
    *(.rodata*)

    . = ALIGN(4);
    KEEP(*(.init))
    KEEP(*(.fini))

    . = ALIGN(4);
    __preinit_array_start = .;
    KEEP(*(.preinit_array))
    __preinit_array_end = .;

    . = ALIGN(4);
    __init_array_start = .;
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array))
    __init_array_end = .;

    . = ALIGN(4);
    __fini_array_start = .;
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array))
    __fini_array_end = .;

    KEEP(*crtbegin.o(.ctors))
    KEEP(*(EXCLUDE_FILE(*crtend.o) .ctors))
    KEEP(*(SORT(.ctors.*)))
    KEEP(*crtend.o(.ctors))
    KEEP(*crtbegin.o(.dtors))
    KEEP(*(EXCLUDE_FILE(*crtend.o) .dtors))
    KEEP(*(SORT(.dtors.*)))
    KEEP(*crtend.o(.dtors))

    . = ALIGN(4);
  } > FLASH

  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } > FLASH

  .ARM.exidx :
  {
    __exidx_start = .;
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    __exidx_end = .;
  } > FLASH

  . = ALIGN(4);
  __etext = .;

  .data : AT (__etext)
  {
    . = ALIGN(4);
    __data_start__ = .;
    *(.ramfunc .ramfunc.*)
    *(.data*)
    . = ALIGN(4);
    __data_end__ = .;
  } > RAM

  .bss (NOLOAD) :
  {
    . = ALIGN(4);
    __bss_start__ = .;
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    __bss_end__ = .;
  } > RAM

  /* Heap grows up from here, stack grows down from the top of RAM */
  . = ALIGN(8);
  __end__ = .;
  end = __end__;
  _end = __end__;

  __StackTop   = ORIGIN(RAM) + LENGTH(RAM);
  __StackTop   = __StackTop & ~7;
  __StackLimit = __StackTop - STACK_SIZE;
  __HeapLimit  = __StackLimit;
  PROVIDE(__stack = __StackTop);

  ASSERT(__StackLimit >= __end__, "RAM overflow: not enough memory for the stack")
}
