/*
 * Gunther.ino - generic port layer (any Arduino core without a dedicated port,
 * e.g. STM32duino for the STM32 Gunther.ino boards)
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 *
 * No addressable LED driver here: a board that has one provides its own port file
 * (or defines LED_RGB as a plain LED pin). The animations run from update().
 */
#if !defined(ARDUINO_ARCH_SAMD51GUNTHER)
#include "Gunther_Port.h"

static bool s_owned = false;

uint8_t gunther_port_builtin_led()
{
#ifdef LED_RGB
  return LED_RGB;
#else
  return GUNTHER_NO_LED;
#endif
}

void gunther_port_builtin_rgb(uint8_t r, uint8_t g, uint8_t b)
{
  uint8_t pin = gunther_port_builtin_led();
  if (pin == GUNTHER_NO_LED) return;
  uint8_t level = r > g ? (r > b ? r : b) : (g > b ? g : b);
  pinMode(pin, OUTPUT);
  digitalWrite(pin, level > 8 ? HIGH : LOW);
}

void gunther_port_rgb(uint8_t pin, uint8_t r, uint8_t g, uint8_t b)
{
  uint8_t level = r > g ? (r > b ? r : b) : (g > b ? g : b);
  pinMode(pin, OUTPUT);
  digitalWrite(pin, level > 8 ? HIGH : LOW);
}

bool gunther_port_sketch_owns_led()       { return s_owned; }
void gunther_port_release_led()           { s_owned = false; }
bool gunther_port_has_pwm(uint8_t pin)    { (void)pin; return true; }
bool gunther_port_start_tick(void (*fn)(void)) { (void)fn; return false; }

float gunther_port_battery()
{
  analogReadResolution(12);
  uint32_t sum = 0;
  for (int i = 0; i < 8; i++) sum += analogRead(VBAT_SENSE);
  return ((float)sum / 8.0f) * 3.3f / 4095.0f * VBAT_DIVIDER_RATIO;
}

void gunther_port_bootloader() { NVIC_SystemReset(); }
void gunther_port_toggle(uint8_t pin) { digitalWrite(pin, !digitalRead(pin)); }

#endif
