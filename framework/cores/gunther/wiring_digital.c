/*
 * Gunther.ino Arduino core - digital I/O, pulseIn, shiftIn/shiftOut
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#include "gunther_internal.h"

void pinMode(uint32_t pin, uint32_t mode)
{
  if (pin >= NUM_DIGITAL_PINS) return;
  if (pin == LED_RGB) return;                     /* built-in RGB LED: always an output */
  PortGroup *g = &PORT->Group[g_pins[pin].port];
  uint32_t bit  = g_pins[pin].pin;
  uint32_t mask = 1UL << bit;

  switch (mode) {
    case OUTPUT:
      g->PINCFG[bit].reg = PORT_PINCFG_INEN;      /* keep input buffer: digitalRead() works on outputs */
      g->DIRSET.reg = mask;
      break;
    case INPUT_PULLUP:
      g->DIRCLR.reg = mask;
      g->PINCFG[bit].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
      g->OUTSET.reg = mask;
      break;
    case INPUT_PULLDOWN:
      g->DIRCLR.reg = mask;
      g->PINCFG[bit].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
      g->OUTCLR.reg = mask;
      break;
    case INPUT:
    default:
      g->DIRCLR.reg = mask;
      g->PINCFG[bit].reg = PORT_PINCFG_INEN;
      break;
  }
}

void digitalWrite(uint32_t pin, uint32_t value)
{
  if (pin >= NUM_DIGITAL_PINS) return;
  if (pin == LED_RGB) { gunther_builtin_digital(value); return; }
  PortGroup *g = &PORT->Group[g_pins[pin].port];
  uint32_t bit  = g_pins[pin].pin;
  uint32_t mask = 1UL << bit;

  /* Arduino behaviour: writing HIGH on an input turns the pull-up on */
  if ((g->DIR.reg & mask) == 0) {
    if (value) g->PINCFG[bit].reg |= PORT_PINCFG_PULLEN;
    else       g->PINCFG[bit].reg &= (uint8_t)~PORT_PINCFG_PULLEN;
  }
  if (value) g->OUTSET.reg = mask;
  else       g->OUTCLR.reg = mask;
}

int digitalRead(uint32_t pin)
{
  if (pin >= NUM_DIGITAL_PINS) return LOW;
  if (pin == LED_RGB) return gunther_builtin_read();
  PortGroup *g = &PORT->Group[g_pins[pin].port];
  return (g->IN.reg & (1UL << g_pins[pin].pin)) ? HIGH : LOW;
}

void digitalToggle(uint32_t pin)
{
  if (pin >= NUM_DIGITAL_PINS) return;
  if (pin == LED_RGB) { gunther_builtin_digital(!gunther_builtin_read()); return; }
  PORT->Group[g_pins[pin].port].OUTTGL.reg = 1UL << g_pins[pin].pin;
}

/* ---------------------------------------------------------------- pulses */
uint32_t pulseInLong(uint32_t pin, uint32_t state, uint32_t timeout_us)
{
  if (pin >= NUM_DIGITAL_PINS) return 0;
  const uint32_t start = micros();
  state = state ? HIGH : LOW;

  while ((uint32_t)digitalRead(pin) == state)        /* wait end of previous pulse */
    if (micros() - start > timeout_us) return 0;
  while ((uint32_t)digitalRead(pin) != state)        /* wait pulse start */
    if (micros() - start > timeout_us) return 0;
  const uint32_t pulse_start = micros();
  while ((uint32_t)digitalRead(pin) == state)        /* wait pulse end */
    if (micros() - start > timeout_us) return 0;
  return micros() - pulse_start;
}

uint32_t pulseIn(uint32_t pin, uint32_t state, uint32_t timeout_us)
{
  return pulseInLong(pin, state, timeout_us);
}

/* ---------------------------------------------------------------- shift */
void shiftOut(uint32_t dataPin, uint32_t clockPin, uint32_t bitOrder, uint8_t val)
{
  for (uint8_t i = 0; i < 8; i++) {
    if (bitOrder == LSBFIRST) digitalWrite(dataPin, (val >> i) & 1u);
    else                      digitalWrite(dataPin, (val >> (7 - i)) & 1u);
    digitalWrite(clockPin, HIGH);
    delayMicroseconds(1);
    digitalWrite(clockPin, LOW);
    delayMicroseconds(1);
  }
}

uint8_t shiftIn(uint32_t dataPin, uint32_t clockPin, uint32_t bitOrder)
{
  uint8_t value = 0;
  for (uint8_t i = 0; i < 8; i++) {
    digitalWrite(clockPin, HIGH);
    delayMicroseconds(1);
    if (bitOrder == LSBFIRST) value |= (uint8_t)(digitalRead(dataPin) << i);
    else                      value |= (uint8_t)(digitalRead(dataPin) << (7 - i));
    digitalWrite(clockPin, LOW);
    delayMicroseconds(1);
  }
  return value;
}
