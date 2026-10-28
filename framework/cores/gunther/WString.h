/*
 * Gunther.ino Arduino core - String class
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#ifndef GUNTHER_WSTRING_H
#define GUNTHER_WSTRING_H

#ifdef __cplusplus

#include <stdint.h>
#include <stddef.h>
#include <string.h>

class __FlashStringHelper;

class String {
public:
  /* ---- construction ---- */
  String(const char *cstr = "");
  String(const char *cstr, size_t length);
  String(const String &other);
  String(String &&other);
  String(const __FlashStringHelper *str) : String((const char *)str) {}
  explicit String(char c);
  explicit String(unsigned char value, unsigned char base = 10);
  explicit String(int value, unsigned char base = 10);
  explicit String(unsigned int value, unsigned char base = 10);
  explicit String(long value, unsigned char base = 10);
  explicit String(unsigned long value, unsigned char base = 10);
  explicit String(long long value, unsigned char base = 10);
  explicit String(unsigned long long value, unsigned char base = 10);
  explicit String(float value, unsigned char decimals = 2);
  explicit String(double value, unsigned char decimals = 2);
  ~String();

  /* ---- memory ---- */
  bool reserve(size_t size);
  size_t length(void) const { return _len; }
  bool isEmpty(void) const { return _len == 0; }
  const char *c_str() const { return _buf ? _buf : ""; }
  char *begin() { return _buf; }
  char *end() { return _buf ? _buf + _len : 0; }
  const char *begin() const { return c_str(); }
  const char *end() const { return c_str() + _len; }

  /* ---- assignment ---- */
  String &operator=(const String &rhs);
  String &operator=(String &&rhs);
  String &operator=(const char *cstr);
  String &operator=(char c) { return *this = String(c); }
  String &operator=(const __FlashStringHelper *str) { return *this = (const char *)str; }

  /* ---- concatenation ---- */
  bool concat(const String &s)       { return append(s.c_str(), s._len); }
  bool concat(const char *cstr)      { return cstr ? append(cstr, strlen(cstr)) : false; }
  bool concat(const char *cstr, size_t n) { return append(cstr, n); }
  bool concat(char c)                { return append(&c, 1); }
  bool concat(unsigned char n)       { return concat(String(n)); }
  bool concat(int n)                 { return concat(String(n)); }
  bool concat(unsigned int n)        { return concat(String(n)); }
  bool concat(long n)                { return concat(String(n)); }
  bool concat(unsigned long n)       { return concat(String(n)); }
  bool concat(long long n)           { return concat(String(n)); }
  bool concat(unsigned long long n)  { return concat(String(n)); }
  bool concat(float n)               { return concat(String(n)); }
  bool concat(double n)              { return concat(String(n)); }
  bool concat(const __FlashStringHelper *s) { return concat((const char *)s); }

  template <typename T> String &operator+=(const T &rhs) { concat(rhs); return *this; }

  /* ---- comparison ---- */
  int  compareTo(const String &s) const;
  bool equals(const String &s) const;
  bool equals(const char *cstr) const;
  bool equalsIgnoreCase(const String &s) const;
  bool startsWith(const String &prefix) const;
  bool startsWith(const String &prefix, size_t offset) const;
  bool endsWith(const String &suffix) const;

  bool operator==(const String &rhs) const { return equals(rhs); }
  bool operator==(const char *cstr) const { return equals(cstr); }
  bool operator!=(const String &rhs) const { return !equals(rhs); }
  bool operator!=(const char *cstr) const { return !equals(cstr); }
  bool operator<(const String &rhs) const  { return compareTo(rhs) < 0; }
  bool operator>(const String &rhs) const  { return compareTo(rhs) > 0; }
  bool operator<=(const String &rhs) const { return compareTo(rhs) <= 0; }
  bool operator>=(const String &rhs) const { return compareTo(rhs) >= 0; }

  /* ---- characters ---- */
  char charAt(size_t index) const;
  void setCharAt(size_t index, char c);
  char operator[](size_t index) const { return charAt(index); }
  char &operator[](size_t index);
  void getBytes(unsigned char *buf, size_t bufsize, size_t index = 0) const;
  void toCharArray(char *buf, size_t bufsize, size_t index = 0) const
  { getBytes((unsigned char *)buf, bufsize, index); }

  /* ---- search ---- */
  int indexOf(char c, size_t from = 0) const;
  int indexOf(const String &s, size_t from = 0) const;
  int lastIndexOf(char c) const;
  int lastIndexOf(char c, size_t from) const;
  int lastIndexOf(const String &s) const;
  int lastIndexOf(const String &s, size_t from) const;
  String substring(size_t beginIndex) const { return substring(beginIndex, _len); }
  String substring(size_t beginIndex, size_t endIndex) const;

  /* ---- modification ---- */
  void replace(char find, char replace);
  void replace(const String &find, const String &replace);
  void remove(size_t index);
  void remove(size_t index, size_t count);
  void toLowerCase(void);
  void toUpperCase(void);
  void trim(void);

  /* ---- conversion ---- */
  long   toInt(void) const;
  float  toFloat(void) const;
  double toDouble(void) const;

  operator bool() const { return _buf != 0; }   /* false only if memory allocation failed */

private:
  char  *_buf;
  size_t _cap;
  size_t _len;
  void init() { _buf = 0; _cap = 0; _len = 0; }
  bool append(const char *s, size_t n);
  String &copy(const char *s, size_t n);
};

/* operator+ for every common combination */
inline String operator+(const String &a, const String &b) { String r(a); r.concat(b); return r; }
inline String operator+(const String &a, const char *b)   { String r(a); r.concat(b); return r; }
inline String operator+(const char *a, const String &b)   { String r(a); r.concat(b); return r; }
inline String operator+(const String &a, char b)          { String r(a); r.concat(b); return r; }
inline String operator+(char a, const String &b)          { String r(a); r.concat(b); return r; }
inline String operator+(const String &a, unsigned char b) { String r(a); r.concat(b); return r; }
inline String operator+(const String &a, int b)           { String r(a); r.concat(b); return r; }
inline String operator+(const String &a, unsigned int b)  { String r(a); r.concat(b); return r; }
inline String operator+(const String &a, long b)          { String r(a); r.concat(b); return r; }
inline String operator+(const String &a, unsigned long b) { String r(a); r.concat(b); return r; }
inline String operator+(const String &a, long long b)     { String r(a); r.concat(b); return r; }
inline String operator+(const String &a, unsigned long long b) { String r(a); r.concat(b); return r; }
inline String operator+(const String &a, float b)         { String r(a); r.concat(b); return r; }
inline String operator+(const String &a, double b)        { String r(a); r.concat(b); return r; }
inline bool operator==(const char *a, const String &b)    { return b.equals(a); }
inline bool operator!=(const char *a, const String &b)    { return !b.equals(a); }

#endif /* __cplusplus */
#endif
