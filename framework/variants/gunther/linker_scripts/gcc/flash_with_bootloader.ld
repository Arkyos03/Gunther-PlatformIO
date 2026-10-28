/*
 * Gunther.ino - linker script for ATSAMD51J20A (1 MB flash, 256 KB RAM)
 * Application placed after the 16 KB UF2 bootloader.
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License
 */

MEMORY
{
  /* 16 KB reserved for the UF2 bootloader, last 8 KB reserved for the EEPROM library */
  FLASH (rx)  : ORIGIN = 0x00004000, LENGTH = 0x00100000 - 0x4000 - 0x2000
  /* last 4 bytes of RAM reserved for the bootloader "double tap" magic value */
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00040000 - 4
}

INCLUDE gunther_sections.ld
