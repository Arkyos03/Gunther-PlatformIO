/*
 * Gunther.ino Arduino core - Stream class (reading with timeout, parsing)
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#ifndef GUNTHER_STREAM_H
#define GUNTHER_STREAM_H

#include "Print.h"

enum LookaheadMode {
  SKIP_ALL,         /* skip everything that is not part of a number */
  SKIP_NONE,        /* stop at the first character that is not a digit */
  SKIP_WHITESPACE   /* only skip spaces, tabs and line breaks */
};

#define NO_IGNORE_CHAR '\x01'

class Stream : public Print {
public:
  Stream() : _timeout(1000), _startMillis(0) {}

  virtual int available() = 0;
  virtual int read() = 0;
  virtual int peek() = 0;

  void setTimeout(unsigned long timeout) { _timeout = timeout; }
  unsigned long getTimeout(void) { return _timeout; }

  bool find(const char *target);
  bool find(const char *target, size_t length);
  bool find(char target) { return find(&target, 1); }
  bool findUntil(const char *target, const char *terminator);

  long  parseInt(LookaheadMode lookahead = SKIP_ALL, char ignore = NO_IGNORE_CHAR);
  float parseFloat(LookaheadMode lookahead = SKIP_ALL, char ignore = NO_IGNORE_CHAR);

  size_t readBytes(char *buffer, size_t length);
  size_t readBytes(uint8_t *buffer, size_t length) { return readBytes((char *)buffer, length); }
  size_t readBytesUntil(char terminator, char *buffer, size_t length);
  size_t readBytesUntil(char terminator, uint8_t *buffer, size_t length)
  { return readBytesUntil(terminator, (char *)buffer, length); }

  String readString();
  String readStringUntil(char terminator);

protected:
  unsigned long _timeout;
  unsigned long _startMillis;
  int timedRead();
  int timedPeek();
  int peekNextDigit(LookaheadMode lookahead, bool detectDecimal);
};

#endif
