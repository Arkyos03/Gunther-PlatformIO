/*
 * Gunther.ino - port layer for the SAMD51 platform (cores/gunther)
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#if defined(ARDUINO_ARCH_SAMD51GUNTHER)
#include "Gunther_Port.h"
#include "gunther_internal.h"

uint8_t gunther_port_builtin_led() { return LED_RGB; }

void gunther_port_builtin_rgb(uint8_t r, uint8_t g, uint8_t b) { gunther_builtin_rgb(r, g, b); }

void gunther_port_rgb(uint8_t pin, uint8_t r, uint8_t g, uint8_t b)
{
  uint8_t grb[3] = { g, r, b };
  gunther_rgb_send(pin, grb, 1);
}

bool gunther_port_sketch_owns_led() { return gunther_builtin_user != 0; }
void gunther_port_release_led()     { gunther_builtin_user = 0; }
bool gunther_port_has_pwm(uint8_t pin) { return pin < NUM_DIGITAL_PINS && g_pins[pin].pwm != NO_PWM; }

bool gunther_port_start_tick(void (*fn)(void))
{
  gunther_led_hook = fn;                      /* PendSV every 20 ms (core) */
  return true;
}

float gunther_port_battery()            { return readBatteryVoltage(); }
void  gunther_port_bootloader()         { resetToBootloader(); }
void  gunther_port_toggle(uint8_t pin)  { digitalToggle(pin); }

#endif
