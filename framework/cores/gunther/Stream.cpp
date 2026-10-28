/*
 * Gunther.ino Arduino core - Stream class
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#include "Arduino.h"
#include "Stream.h"

int Stream::timedRead()
{
  _startMillis = millis();
  do {
    int c = read();
    if (c >= 0) return c;
    yield();
  } while (millis() - _startMillis < _timeout);
  return -1;
}

int Stream::timedPeek()
{
  _startMillis = millis();
  do {
    int c = peek();
    if (c >= 0) return c;
    yield();
  } while (millis() - _startMillis < _timeout);
  return -1;
}

int Stream::peekNextDigit(LookaheadMode lookahead, bool detectDecimal)
{
  while (true) {
    int c = timedPeek();
    if (c < 0) return c;
    if (c == '-' || (c >= '0' && c <= '9') || (detectDecimal && c == '.')) return c;
    switch (lookahead) {
      case SKIP_NONE: return -1;
      case SKIP_WHITESPACE:
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n') return -1;
        break;
      case SKIP_ALL:
      default:
        break;
    }
    read();
  }
}

bool Stream::find(const char *target)
{
  return find(target, strlen(target));
}

bool Stream::find(const char *target, size_t length)
{
  if (length == 0) return true;
  size_t matched = 0;
  int c;
  while ((c = timedRead()) >= 0) {
    if (c == target[matched]) {
      if (++matched == length) return true;
    } else {
      /* restart, the current char may be the beginning of the target */
      matched = (c == target[0]) ? 1 : 0;
      if (matched == length) return true;
    }
  }
  return false;
}

bool Stream::findUntil(const char *target, const char *terminator)
{
  size_t tlen = strlen(target), endlen = terminator ? strlen(terminator) : 0;
  size_t ti = 0, ei = 0;
  int c;
  while ((c = timedRead()) >= 0) {
    if (tlen && c == target[ti]) { if (++ti == tlen) return true; }
    else ti = (tlen && c == target[0]) ? 1 : 0;
    if (endlen) {
      if (c == terminator[ei]) { if (++ei == endlen) return false; }
      else ei = (c == terminator[0]) ? 1 : 0;
    }
  }
  return false;
}

long Stream::parseInt(LookaheadMode lookahead, char ignore)
{
  bool negative = false;
  long value = 0;
  int c = peekNextDigit(lookahead, false);
  if (c < 0) return 0;
  do {
    if (c == ignore) { }
    else if (c == '-') negative = true;
    else if (c >= '0' && c <= '9') value = value * 10 + (c - '0');
    read();
    c = timedPeek();
  } while ((c >= '0' && c <= '9') || c == ignore);
  return negative ? -value : value;
}

float Stream::parseFloat(LookaheadMode lookahead, char ignore)
{
  bool negative = false, fraction = false;
  double value = 0.0, scale = 1.0;
  int c = peekNextDigit(lookahead, true);
  if (c < 0) return 0.0f;
  do {
    if (c == ignore) { }
    else if (c == '-') negative = true;
    else if (c == '.') fraction = true;
    else if (c >= '0' && c <= '9') {
      if (fraction) { scale *= 0.1; value += (c - '0') * scale; }
      else value = value * 10.0 + (c - '0');
    }
    read();
    c = timedPeek();
  } while ((c >= '0' && c <= '9') || (c == '.' && !fraction) || c == ignore);
  return (float)(negative ? -value : value);
}

size_t Stream::readBytes(char *buffer, size_t length)
{
  size_t count = 0;
  while (count < length) {
    int c = timedRead();
    if (c < 0) break;
    *buffer++ = (char)c;
    count++;
  }
  return count;
}

size_t Stream::readBytesUntil(char terminator, char *buffer, size_t length)
{
  size_t count = 0;
  while (count < length) {
    int c = timedRead();
    if (c < 0 || c == terminator) break;
    *buffer++ = (char)c;
    count++;
  }
  return count;
}

String Stream::readString()
{
  String s;
  int c;
  while ((c = timedRead()) >= 0) s += (char)c;
  return s;
}

String Stream::readStringUntil(char terminator)
{
  String s;
  int c;
  while ((c = timedRead()) >= 0 && c != terminator) s += (char)c;
  return s;
}
