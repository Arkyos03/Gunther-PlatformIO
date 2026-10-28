/*
 * Gunther.ino Arduino core - math helpers, dtostrf, default overloads
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#include "Arduino.h"
#include <stdio.h>

/* Small xorshift generator: deterministic unless randomSeed() is called */
static uint32_t s_rand_state = 2463534242UL;

static uint32_t next_rand(void)
{
  uint32_t x = s_rand_state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  s_rand_state = x;
  return x;
}

void randomSeed(unsigned long seed)
{
  if (seed != 0) s_rand_state = (uint32_t)seed;
}

long random(long howbig)
{
  if (howbig <= 0) return 0;
  return (long)(next_rand() % (uint32_t)howbig);
}

long random(long howsmall, long howbig)
{
  if (howsmall >= howbig) return howsmall;
  return random(howbig - howsmall) + howsmall;
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  if (in_max == in_min) return out_min;
  return (long)(((int64_t)(x - in_min) * (out_max - out_min)) / (in_max - in_min)) + out_min;
}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  if (in_max == in_min) return out_min;
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

uint16_t makeWord(uint16_t w)            { return w; }
uint16_t makeWord(uint8_t h, uint8_t l)  { return (uint16_t)((h << 8) | l); }

uint32_t pulseIn(uint32_t pin, uint32_t state)     { return pulseInLong(pin, state, 1000000UL); }
uint32_t pulseInLong(uint32_t pin, uint32_t state) { return pulseInLong(pin, state, 1000000UL); }
void tone(uint32_t pin, uint32_t frequency)        { tone(pin, frequency, 0); }

extern "C" char *dtostrf(double val, signed char width, unsigned char prec, char *sout)
{
  char fmt[16];
  snprintf(fmt, sizeof(fmt), "%%%d.%df", (int)width, (int)prec);
  sprintf(sout, fmt, val);
  return sout;
}
