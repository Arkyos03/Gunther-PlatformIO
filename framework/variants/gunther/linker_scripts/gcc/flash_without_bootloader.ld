/*
 * Gunther.ino - linker script for ATSAMD51J20A, no bootloader (flash through SWD)
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License
 */

MEMORY
{
  /* last 8 KB reserved for the EEPROM library */
  FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 0x00100000 - 0x2000
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00040000 - 4
}

INCLUDE gunther_sections.ld
