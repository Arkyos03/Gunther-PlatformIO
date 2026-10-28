/*
 * Gunther.ino - Servo library
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#include "Servo.h"
#include "gunther_internal.h"

Servo::Servo() : _pin(-1), _cc(0), _min(MIN_PULSE_WIDTH), _max(MAX_PULSE_WIDTH), _us(DEFAULT_PULSE_WIDTH) {}

uint8_t Servo::attach(int pin)
{
  return attach(pin, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);
}

uint8_t Servo::attach(int pin, int min_us, int max_us)
{
  if (pin < 0 || pin >= (int)NUM_DIGITAL_PINS) return INVALID_SERVO;
  uint8_t pwm = g_pins[pin].pwm;
  if (pwm == NO_PWM || (pwm >> 4) != GUNTHER_SERVO_TCC) return INVALID_SERVO;

  if (min_us < 200) min_us = 200;
  if (max_us > 3000) max_us = 3000;
  if (min_us >= max_us) { min_us = MIN_PULSE_WIDTH; max_us = MAX_PULSE_WIDTH; }

  _pin = (int8_t)pin;
  _cc  = (uint8_t)((pwm & 0x0F) % TCC1_CC_NUM);
  _min = (uint16_t)min_us;
  _max = (uint16_t)max_us;

  gunther_tcc_init(GUNTHER_SERVO_TCC);
  writeMicroseconds(_us);
  PORT->Group[g_pins[pin].port].DIRSET.reg = 1UL << g_pins[pin].pin;
  gunther_pin_set_mux((uint32_t)pin, g_pins[pin].pwm_mux);
  return _cc;
}

void Servo::detach()
{
  if (_pin < 0) return;
  gunther_tcc_write(GUNTHER_SERVO_TCC, _cc, 0);
  gunther_pin_set_gpio((uint32_t)_pin);
  digitalWrite((uint32_t)_pin, LOW);
  _pin = -1;
}

void Servo::write(int value)
{
  if (value < 200) {               /* angle */
    if (value < 0)   value = 0;
    if (value > 180) value = 180;
    value = (int)map(value, 0, 180, _min, _max);
  }
  writeMicroseconds(value);
}

void Servo::writeMicroseconds(int us)
{
  if (us < _min) us = _min;
  if (us > _max) us = _max;
  _us = (uint16_t)us;
  if (_pin >= 0) gunther_tcc_write(GUNTHER_SERVO_TCC, _cc, (uint32_t)us * GUNTHER_SERVO_TICKS_US);
}

int Servo::read()
{
  return (int)map(_us, _min, _max, 0, 180);
}

int Servo::readMicroseconds()
{
  return _us;
}

bool Servo::attached()
{
  return _pin >= 0;
}
