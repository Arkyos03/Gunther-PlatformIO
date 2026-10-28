/*
 * Gunther.ino Arduino core - String class
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "WString.h"

extern "C" char *dtostrf(double val, signed char width, unsigned char prec, char *sout);

/* --------------------------------------------------------------- helpers */
static void utoa_base(unsigned long long v, unsigned base, char *out)
{
  char tmp[66];
  int i = 0;
  if (base < 2 || base > 36) base = 10;
  do {
    unsigned d = (unsigned)(v % base);
    tmp[i++] = (char)(d < 10 ? '0' + d : 'a' + d - 10);
    v /= base;
  } while (v);
  int j = 0;
  while (i) out[j++] = tmp[--i];
  out[j] = '\0';
}

static void stoa_base(long long v, unsigned base, char *out)
{
  if (base == 10 && v < 0) {
    out[0] = '-';
    utoa_base((unsigned long long)(-(v + 1)) + 1ULL, 10, out + 1);
  } else if (v < 0 && v >= -2147483648LL) {
    utoa_base((unsigned long)v, base, out);        /* 32-bit two's complement */
  } else {
    utoa_base((unsigned long long)v, base, out);
  }
}

bool String::reserve(size_t size)
{
  if (_buf && _cap >= size) return true;
  char *nb = (char *)realloc(_buf, size + 1);
  if (!nb) return false;
  if (!_buf) nb[0] = '\0';
  _buf = nb;
  _cap = size;
  return true;
}

String &String::copy(const char *s, size_t n)
{
  if (!reserve(n)) {
    free(_buf);
    init();
    return *this;
  }
  memmove(_buf, s, n);
  _buf[n] = '\0';
  _len = n;
  return *this;
}

bool String::append(const char *s, size_t n)
{
  if (!s) return false;
  if (n == 0) return true;
  size_t newlen = _len + n;
  /* s may point inside our own buffer: remember its offset */
  ptrdiff_t self = (_buf && s >= _buf && s < _buf + _len) ? (s - _buf) : -1;
  if (!reserve(newlen + (newlen >> 2))) {      /* grow by 25 % to limit reallocations */
    if (!reserve(newlen)) return false;
  }
  if (self >= 0) s = _buf + self;
  memmove(_buf + _len, s, n);
  _len = newlen;
  _buf[_len] = '\0';
  return true;
}

/* ---------------------------------------------------------- constructors */
String::String(const char *cstr)           { init(); if (cstr) copy(cstr, strlen(cstr)); }
String::String(const char *cstr, size_t n) { init(); if (cstr) copy(cstr, n); }
String::String(const String &o)            { init(); copy(o.c_str(), o._len); }
String::String(String &&o)                 { _buf = o._buf; _cap = o._cap; _len = o._len; o.init(); }
String::String(char c)                     { init(); copy(&c, 1); }

String::String(unsigned char v, unsigned char base) { init(); char b[66]; utoa_base(v, base, b); copy(b, strlen(b)); }
String::String(unsigned int v, unsigned char base)  { init(); char b[66]; utoa_base(v, base, b); copy(b, strlen(b)); }
String::String(unsigned long v, unsigned char base) { init(); char b[66]; utoa_base(v, base, b); copy(b, strlen(b)); }
String::String(unsigned long long v, unsigned char base) { init(); char b[66]; utoa_base(v, base, b); copy(b, strlen(b)); }
String::String(int v, unsigned char base)       { init(); char b[67]; stoa_base(v, base, b); copy(b, strlen(b)); }
String::String(long v, unsigned char base)      { init(); char b[67]; stoa_base(v, base, b); copy(b, strlen(b)); }
String::String(long long v, unsigned char base) { init(); char b[67]; stoa_base(v, base, b); copy(b, strlen(b)); }

String::String(float v, unsigned char decimals)  : String((double)v, decimals) {}
String::String(double v, unsigned char decimals)
{
  init();
  char b[48];
  if (decimals > 9) decimals = 9;
  dtostrf(v, 1, decimals, b);
  copy(b, strlen(b));
}

String::~String() { free(_buf); }

/* ------------------------------------------------------------ assignment */
String &String::operator=(const String &rhs)
{
  if (this != &rhs) copy(rhs.c_str(), rhs._len);
  return *this;
}

String &String::operator=(String &&rhs)
{
  if (this != &rhs) {
    free(_buf);
    _buf = rhs._buf; _cap = rhs._cap; _len = rhs._len;
    rhs.init();
  }
  return *this;
}

String &String::operator=(const char *cstr)
{
  if (cstr) copy(cstr, strlen(cstr));
  else { free(_buf); init(); }
  return *this;
}

/* ------------------------------------------------------------ comparison */
int String::compareTo(const String &s) const
{
  return strcmp(c_str(), s.c_str());
}

bool String::equals(const String &s) const
{
  return _len == s._len && memcmp(c_str(), s.c_str(), _len) == 0;
}

bool String::equals(const char *cstr) const
{
  if (!cstr) return _len == 0;
  return strcmp(c_str(), cstr) == 0;
}

bool String::equalsIgnoreCase(const String &s) const
{
  if (_len != s._len) return false;
  const char *a = c_str(), *b = s.c_str();
  for (size_t i = 0; i < _len; i++)
    if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i])) return false;
  return true;
}

bool String::startsWith(const String &p) const { return startsWith(p, 0); }

bool String::startsWith(const String &p, size_t offset) const
{
  if (offset > _len || p._len > _len - offset) return false;
  return memcmp(c_str() + offset, p.c_str(), p._len) == 0;
}

bool String::endsWith(const String &s) const
{
  if (s._len > _len) return false;
  return memcmp(c_str() + _len - s._len, s.c_str(), s._len) == 0;
}

/* ------------------------------------------------------------ characters */
char String::charAt(size_t i) const { return (i < _len) ? _buf[i] : '\0'; }

void String::setCharAt(size_t i, char c) { if (i < _len) _buf[i] = c; }

char &String::operator[](size_t i)
{
  static char dummy;
  if (i >= _len) { dummy = '\0'; return dummy; }
  return _buf[i];
}

void String::getBytes(unsigned char *buf, size_t bufsize, size_t index) const
{
  if (!bufsize || !buf) return;
  if (index >= _len) { buf[0] = 0; return; }
  size_t n = bufsize - 1;
  if (n > _len - index) n = _len - index;
  memcpy(buf, _buf + index, n);
  buf[n] = 0;
}

/* ---------------------------------------------------------------- search */
int String::indexOf(char c, size_t from) const
{
  if (from >= _len) return -1;
  const char *p = (const char *)memchr(_buf + from, c, _len - from);
  return p ? (int)(p - _buf) : -1;
}

int String::indexOf(const String &s, size_t from) const
{
  if (from > _len) return -1;
  const char *p = strstr(c_str() + from, s.c_str());
  return p ? (int)(p - c_str()) : -1;
}

int String::lastIndexOf(char c) const { return _len ? lastIndexOf(c, _len - 1) : -1; }

int String::lastIndexOf(char c, size_t from) const
{
  if (!_len) return -1;
  if (from >= _len) from = _len - 1;
  for (int i = (int)from; i >= 0; i--) if (_buf[i] == c) return i;
  return -1;
}

int String::lastIndexOf(const String &s) const { return lastIndexOf(s, _len); }

int String::lastIndexOf(const String &s, size_t from) const
{
  if (s._len == 0 || s._len > _len) return -1;
  if (from > _len - s._len) from = _len - s._len;
  for (int i = (int)from; i >= 0; i--)
    if (memcmp(_buf + i, s.c_str(), s._len) == 0) return i;
  return -1;
}

String String::substring(size_t b, size_t e) const
{
  if (b > e) { size_t t = b; b = e; e = t; }
  if (b >= _len) return String();
  if (e > _len) e = _len;
  return String(_buf + b, e - b);
}

/* ---------------------------------------------------------- modification */
void String::replace(char find, char repl)
{
  for (size_t i = 0; i < _len; i++) if (_buf[i] == find) _buf[i] = repl;
}

void String::replace(const String &find, const String &repl)
{
  if (_len == 0 || find._len == 0) return;
  String out;
  out.reserve(_len);
  size_t i = 0;
  while (i < _len) {
    if (i + find._len <= _len && memcmp(_buf + i, find.c_str(), find._len) == 0) {
      out.concat(repl);
      i += find._len;
    } else {
      out.concat(_buf[i++]);
    }
  }
  *this = (String &&)out;
}

void String::remove(size_t index) { remove(index, (size_t)-1); }

void String::remove(size_t index, size_t count)
{
  if (index >= _len || count == 0) return;
  if (count > _len - index) count = _len - index;
  memmove(_buf + index, _buf + index + count, _len - index - count + 1);
  _len -= count;
}

void String::toLowerCase(void) { for (size_t i = 0; i < _len; i++) _buf[i] = (char)tolower((unsigned char)_buf[i]); }
void String::toUpperCase(void) { for (size_t i = 0; i < _len; i++) _buf[i] = (char)toupper((unsigned char)_buf[i]); }

void String::trim(void)
{
  if (!_len) return;
  size_t b = 0, e = _len;
  while (b < e && isspace((unsigned char)_buf[b])) b++;
  while (e > b && isspace((unsigned char)_buf[e - 1])) e--;
  _len = e - b;
  memmove(_buf, _buf + b, _len);
  _buf[_len] = '\0';
}

/* ------------------------------------------------------------ conversion */
long   String::toInt(void) const    { return atol(c_str()); }
float  String::toFloat(void) const  { return (float)atof(c_str()); }
double String::toDouble(void) const { return atof(c_str()); }
