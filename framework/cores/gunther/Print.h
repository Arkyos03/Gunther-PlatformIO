/*
 * Gunther.ino Arduino core - Print class (print / println / printf)
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#ifndef GUNTHER_PRINT_H
#define GUNTHER_PRINT_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "WString.h"

class Print;

/* Objects that know how to print themselves */
class Printable {
public:
  virtual size_t printTo(Print &p) const = 0;
  virtual ~Printable() {}
};

class __FlashStringHelper;

class Print {
public:
  Print() : write_error(0) {}
  virtual ~Print() {}

  virtual size_t write(uint8_t c) = 0;
  virtual size_t write(const uint8_t *buffer, size_t size);
  size_t write(const char *str) { return str ? write((const uint8_t *)str, strlen(str)) : 0; }
  size_t write(const char *buffer, size_t size) { return write((const uint8_t *)buffer, size); }
  /* write(0x00) & co: avoid the ambiguity with write(const char *) */
  size_t write(int n)           { return write((uint8_t)n); }
  size_t write(unsigned int n)  { return write((uint8_t)n); }
  size_t write(long n)          { return write((uint8_t)n); }
  size_t write(unsigned long n) { return write((uint8_t)n); }
  size_t write(char c)          { return write((uint8_t)c); }

  virtual int  availableForWrite() { return 0; }
  virtual void flush() {}

  int  getWriteError() { return write_error; }
  void clearWriteError() { write_error = 0; }

  size_t print(const char *s);
  size_t print(const String &s);
  size_t print(const __FlashStringHelper *s) { return print((const char *)s); }
  size_t print(char c);
  size_t print(unsigned char n, int base = 10);
  size_t print(int n, int base = 10);
  size_t print(unsigned int n, int base = 10);
  size_t print(long n, int base = 10);
  size_t print(unsigned long n, int base = 10);
  size_t print(long long n, int base = 10);
  size_t print(unsigned long long n, int base = 10);
  size_t print(double n, int digits = 2);
  size_t print(const Printable &p) { return p.printTo(*this); }

  size_t println(void);
  size_t println(const char *s)                 { return print(s) + println(); }
  size_t println(const String &s)               { return print(s) + println(); }
  size_t println(const __FlashStringHelper *s)  { return print(s) + println(); }
  size_t println(char c)                        { return print(c) + println(); }
  size_t println(unsigned char n, int base = 10){ return print(n, base) + println(); }
  size_t println(int n, int base = 10)          { return print(n, base) + println(); }
  size_t println(unsigned int n, int base = 10) { return print(n, base) + println(); }
  size_t println(long n, int base = 10)         { return print(n, base) + println(); }
  size_t println(unsigned long n, int base = 10){ return print(n, base) + println(); }
  size_t println(long long n, int base = 10)    { return print(n, base) + println(); }
  size_t println(unsigned long long n, int base = 10) { return print(n, base) + println(); }
  size_t println(double n, int digits = 2)      { return print(n, digits) + println(); }
  size_t println(const Printable &p)            { return print(p) + println(); }

  /* printf("%d %.2f\n", a, b) - floats are supported */
  size_t printf(const char *format, ...) __attribute__((format(printf, 2, 3)));
  size_t vprintf(const char *format, va_list args);

protected:
  void setWriteError(int err = 1) { write_error = err; }

private:
  int write_error;
  size_t printUnsigned(unsigned long long n, int base);
  size_t printSigned(long long n, int base);
  size_t printFloat(double n, int digits);
};

#endif
