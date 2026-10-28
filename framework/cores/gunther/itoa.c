/*
 * Gunther.ino Arduino core - ltoa / ultoa (itoa and utoa come from newlib)
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#include "Arduino.h"

char *ultoa(unsigned long value, char *str, int base)
{
  char tmp[33];
  int i = 0;
  if (base < 2 || base > 36) { str[0] = '\0'; return str; }
  do {
    unsigned long d = value % (unsigned long)base;
    tmp[i++] = (char)(d < 10 ? '0' + d : 'a' + d - 10);
    value /= (unsigned long)base;
  } while (value && i < 32);
  int j = 0;
  while (i) str[j++] = tmp[--i];
  str[j] = '\0';
  return str;
}

char *ltoa(long value, char *str, int base)
{
  if (base == 10 && value < 0) {
    str[0] = '-';
    ultoa((unsigned long)(-(value + 1)) + 1UL, str + 1, base);
    return str;
  }
  return ultoa((unsigned long)value, str, base);
}
