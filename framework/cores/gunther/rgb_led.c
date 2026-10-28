/*
 * Gunther.ino Arduino core - addressable RGB LED (SK6805 / WS2812 / SK6812, GRB order)
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 *
 * The built-in LED (LED_BUILTIN = LED_RGB, PA08) is an SK6805-EC15 behind a
 * 74AHCT1G125 level shifter. It also answers digitalWrite / analogWrite like a
 * normal LED: HIGH = white, LOW = off.
 *
 * Timing generated with the cycle counter (120 MHz): 1.25 us per bit,
 * "0" = 0.3 us high, "1" = 0.7 us high, reset > 80 us. Interrupts are off
 * for about 30 us per LED.
 */
#include "gunther_internal.h"

#define RGB_T_BIT   (F_CPU / 800000UL)          /* 1.25 us */
#define RGB_T_0H    (F_CPU / 3333333UL)         /* 0.30 us */
#define RGB_T_1H    (F_CPU / 1428571UL)         /* 0.70 us */
#define RGB_RESET_US 100u

static uint32_t s_last_us[NUM_DIGITAL_PINS > 64 ? 64 : NUM_DIGITAL_PINS];

/* Built-in LED state */
static uint8_t  s_builtin_r, s_builtin_g, s_builtin_b;
static uint8_t  s_builtin_on;
volatile uint8_t gunther_builtin_user = 0;          /* set when the sketch drives the LED itself */

void gunther_rgb_send(uint32_t pin, const uint8_t *grb, uint32_t count)
{
  if (pin >= NUM_DIGITAL_PINS) return;
  const uint32_t port = g_pins[pin].port;
  const uint32_t mask = 1UL << g_pins[pin].pin;
  volatile uint32_t *set = &PORT->Group[port].OUTSET.reg;
  volatile uint32_t *clr = &PORT->Group[port].OUTCLR.reg;

  /* pin = output, GPIO, low */
  PortGroup *g = &PORT->Group[port];
  if ((g->DIR.reg & mask) == 0 || (g->PINCFG[g_pins[pin].pin].reg & PORT_PINCFG_PMUXEN)) {
    g->PINCFG[g_pins[pin].pin].reg = PORT_PINCFG_INEN;
    g->OUTCLR.reg = mask;
    g->DIRSET.reg = mask;
    delayMicroseconds(RGB_RESET_US);
  }

  /* latch time of the previous frame */
  if (pin < 64) {
    while ((micros() - s_last_us[pin]) < RGB_RESET_US) { }
  }

  uint32_t primask = __get_PRIMASK();
  __disable_irq();
  uint32_t t = DWT->CYCCNT - RGB_T_BIT;
  for (uint32_t i = 0; i < count * 3u; i++) {
    uint8_t byte = grb[i];
    for (uint32_t b = 0; b < 8; b++) {
      while ((DWT->CYCCNT - t) < RGB_T_BIT) { }
      t = DWT->CYCCNT;
      *set = mask;
      uint32_t high = (byte & 0x80u) ? RGB_T_1H : RGB_T_0H;
      byte <<= 1;
      while ((DWT->CYCCNT - t) < high) { }
      *clr = mask;
    }
  }
  while ((DWT->CYCCNT - t) < RGB_T_BIT) { }
  __set_PRIMASK(primask);
  if (pin < 64) s_last_us[pin] = micros();
}

/* Used by the core and the Gunther library: does not mark the LED as "used by the sketch" */
void gunther_builtin_rgb(uint8_t r, uint8_t g, uint8_t b)
{
  uint8_t grb[3] = { g, r, b };
  s_builtin_r = r; s_builtin_g = g; s_builtin_b = b;
  s_builtin_on = (r | g | b) ? 1 : 0;
  gunther_rgb_send(LED_RGB, grb, 1);
}

void rgbLedWrite(uint32_t pin, uint8_t red, uint8_t green, uint8_t blue)
{
  if (pin == LED_RGB) {
    gunther_builtin_user = 1;
    gunther_builtin_rgb(red, green, blue);
    return;
  }
  uint8_t grb[3] = { green, red, blue };
  gunther_rgb_send(pin, grb, 1);
}

void rgbLedWriteColor(uint32_t pin, uint32_t rgb)
{
  rgbLedWrite(pin, (uint8_t)(rgb >> 16), (uint8_t)(rgb >> 8), (uint8_t)rgb);
}

void rgbStripWrite(uint32_t pin, const uint8_t *rgb, uint32_t count)
{
  /* rgb = R, G, B, R, G, B ... (64 LEDs max) -> sent in G, R, B order */
  if (count > 64) count = 64;
  if (pin == LED_RGB) gunther_builtin_user = 1;
  uint8_t buf[3 * 64];
  for (uint32_t i = 0; i < count; i++) {
    buf[3 * i]     = rgb[3 * i + 1];
    buf[3 * i + 1] = rgb[3 * i];
    buf[3 * i + 2] = rgb[3 * i + 2];
  }
  gunther_rgb_send(pin, buf, count);
}

uint32_t rgbLedColor(void)
{
  return ((uint32_t)s_builtin_r << 16) | ((uint32_t)s_builtin_g << 8) | s_builtin_b;
}

/* digitalWrite / analogWrite / digitalRead on LED_BUILTIN */
void gunther_builtin_digital(uint32_t value)
{
  gunther_builtin_user = 1;
  if (value) gunther_builtin_rgb(LED_BUILTIN_LEVEL, LED_BUILTIN_LEVEL, LED_BUILTIN_LEVEL);
  else       gunther_builtin_rgb(0, 0, 0);
}

void gunther_builtin_level(uint8_t level)
{
  gunther_builtin_user = 1;
  gunther_builtin_rgb(level, level, level);
}

int gunther_builtin_read(void)
{
  return s_builtin_on ? HIGH : LOW;
}
