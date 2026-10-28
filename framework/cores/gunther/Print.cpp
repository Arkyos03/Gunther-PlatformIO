/*
 * Gunther.ino Arduino core - Print class
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "Print.h"

size_t Print::write(const uint8_t *buffer, size_t size)
{
  size_t n = 0;
  while (size--) {
    if (write(*buffer++)) n++;
    else break;
  }
  return n;
}

size_t Print::print(const char *s)        { return write(s); }
size_t Print::print(const String &s)      { return write(s.c_str(), s.length()); }
size_t Print::print(char c)               { return write((uint8_t)c); }
size_t Print::print(unsigned char n, int base)  { return printUnsigned(n, base); }
size_t Print::print(unsigned int n, int base)   { return printUnsigned(n, base); }
size_t Print::print(unsigned long n, int base)  { return printUnsigned(n, base); }
size_t Print::print(unsigned long long n, int base) { return printUnsigned(n, base); }
size_t Print::print(int n, int base)      { return printSigned(n, base); }
size_t Print::print(long n, int base)     { return printSigned(n, base); }
size_t Print::print(long long n, int base){ return printSigned(n, base); }
size_t Print::print(double n, int digits) { return printFloat(n, digits); }

size_t Print::println(void)
{
  return write("\r\n");
}

size_t Print::printUnsigned(unsigned long long n, int base)
{
  if (base == 0) return write((uint8_t)n);     /* Arduino: base 0 = raw byte */
  if (base < 2 || base > 36) base = 10;
  char buf[65];
  char *p = &buf[sizeof(buf) - 1];
  *p = '\0';
  do {
    unsigned d = (unsigned)(n % (unsigned)base);
    *--p = (char)(d < 10 ? '0' + d : 'A' + d - 10);
    n /= (unsigned)base;
  } while (n);
  return write(p);
}

size_t Print::printSigned(long long n, int base)
{
  if (base == 0) return write((uint8_t)n);
  if (base == 10 && n < 0) {
    size_t t = write((uint8_t)'-');
    return t + printUnsigned((unsigned long long)(-(n + 1)) + 1ULL, 10);
  }
  /* other bases: two's complement, 32 bits for values that fit (Arduino behaviour) */
  if (n >= -2147483648LL && n < 0) return printUnsigned((unsigned long)n, base);
  return printUnsigned((unsigned long long)n, base);
}

size_t Print::printFloat(double number, int digits)
{
  if (isnan(number)) return write("nan");
  if (isinf(number)) return write(number < 0 ? "-inf" : "inf");
  if (number > 4294967040.0 || number < -4294967040.0) return write("ovf");
  if (digits < 0) digits = 0;
  if (digits > 9) digits = 9;

  size_t n = 0;
  if (number < 0.0) { n += write((uint8_t)'-'); number = -number; }

  double rounding = 0.5;
  for (int i = 0; i < digits; i++) rounding /= 10.0;
  number += rounding;

  unsigned long int_part = (unsigned long)number;
  double rem = number - (double)int_part;
  n += printUnsigned(int_part, 10);

  if (digits > 0) {
    n += write((uint8_t)'.');
    while (digits-- > 0) {
      rem *= 10.0;
      unsigned d = (unsigned)rem;
      n += write((uint8_t)('0' + d));
      rem -= d;
    }
  }
  return n;
}

size_t Print::vprintf(const char *format, va_list args)
{
  char small[128];
  va_list copy;
  va_copy(copy, args);
  int len = vsnprintf(small, sizeof(small), format, copy);
  va_end(copy);
  if (len < 0) return 0;
  if ((size_t)len < sizeof(small)) return write((const uint8_t *)small, (size_t)len);

  char *big = new char[len + 1];
  if (!big) return 0;
  vsnprintf(big, (size_t)len + 1, format, args);
  size_t n = write((const uint8_t *)big, (size_t)len);
  delete[] big;
  return n;
}

size_t Print::printf(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  size_t n = vprintf(format, args);
  va_end(args);
  return n;
}
