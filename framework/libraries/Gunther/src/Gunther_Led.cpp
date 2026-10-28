/*
 * Gunther.ino - status animations on the built-in RGB LED
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 *
 * The animation runs every 20 ms. When the platform can (SAMD51: low priority
 * interrupt), it keeps going during delay(), while(true) loops and blocking
 * functions; otherwise it is refreshed by update().
 */
#include "Gunther.h"
#include "Gunther_Port.h"

#define C_BLUE    0x0040FFu
#define C_GREEN   0x00FF20u
#define C_CYAN    0x00E0FFu
#define C_RED     0xFF0000u
#define C_ORANGE  0xFF6000u
#define C_PURPLE  0xB000FFu
#define C_WHITE   0xFFFFFFu

static void ledHook() { Gunther._ledTask(); }

void GuntherBoard::ledStartTask()
{
  _ledTick = gunther_port_start_tick(ledHook);
}

void GuntherBoard::statusLed(bool on)
{
  _ledAuto = on;
  if (!on) {
    ledShow(0, 0, 0);
  } else {
    gunther_port_release_led();                         /* gives the built-in LED back to the animations */
    _lastShown = 0xFFFFFFFF;
    ledStartTask();
  }
}

void GuntherBoard::setStatusLed(uint8_t pin, bool addressable)
{
  bool wasAuto = _ledAuto;
  _ledAuto = false;
  ledShow(0, 0, 0);                                 /* old LED off */
  _ledPin = pin;
  _ledAddressable = addressable;
  _lastShown = 0xFFFFFFFF;
  if (!addressable) pinMode(pin, OUTPUT);
  _ledAuto = wasAuto;
}

void GuntherBoard::statusBrightness(uint8_t level)
{
  _ledBright = level;
}

void GuntherBoard::statusFlash(uint32_t rgb, uint16_t ms)
{
  _flashColor = rgb;
  _flashUntil = millis() + ms;
}

void GuntherBoard::rgb(uint8_t r, uint8_t g, uint8_t b)
{
  if (gunther_port_builtin_led() == GUNTHER_NO_LED) return;
#if defined(ARDUINO_ARCH_SAMD51GUNTHER)
  rgbLedWrite(gunther_port_builtin_led(), r, g, b);  /* the sketch now owns the built-in LED */
#else
  _ledAuto = false;                                  /* no ownership flag in the core: stop the animations */
  gunther_port_builtin_rgb(r, g, b);
#endif
}

void GuntherBoard::rgb(uint32_t c)
{
  rgb((uint8_t)(c >> 16), (uint8_t)(c >> 8), (uint8_t)c);
}

/* Shows a colour on the status LED (built-in or external) */
void GuntherBoard::ledShow(uint8_t r, uint8_t g, uint8_t b)
{
  uint32_t c = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
  if (c == _lastShown) return;
  _lastShown = c;
  if (_ledPin == GUNTHER_NO_LED) return;
  if (_ledPin == gunther_port_builtin_led()) {
    gunther_port_builtin_rgb(r, g, b);
  } else if (_ledAddressable) {
    gunther_port_rgb(_ledPin, r, g, b);
  } else {
    uint8_t level = r > g ? (r > b ? r : b) : (g > b ? g : b);
    if (gunther_port_has_pwm(_ledPin)) analogWrite(_ledPin, level);
    else digitalWrite(_ledPin, level > 8 ? HIGH : LOW);
  }
}

static uint32_t scale(uint32_t c, float k)
{
  if (k < 0.0f) k = 0.0f;
  if (k > 1.0f) k = 1.0f;
  uint32_t r = (uint32_t)(((c >> 16) & 0xFF) * k);
  uint32_t g = (uint32_t)(((c >> 8) & 0xFF) * k);
  uint32_t b = (uint32_t)((c & 0xFF) * k);
  return (r << 16) | (g << 8) | b;
}

static float breathe(uint32_t now, uint32_t period)
{
  float x = (float)(now % period) / (float)period;
  return 0.08f + 0.92f * (0.5f - 0.5f * cosf(2.0f * (float)M_PI * x));
}

static float blink(uint32_t now, uint32_t period, uint32_t on)
{
  return (now % period) < on ? 1.0f : 0.0f;
}

void GuntherBoard::_ledTask()
{
  if (!_ledAuto) return;
  if (_ledPin == gunther_port_builtin_led() && gunther_port_sketch_owns_led()) return;   /* the sketch drives the LED itself */
  uint32_t now = millis();
  uint32_t c;

  if ((int32_t)(_flashUntil - now) > 0) {
    c = _flashColor;                                     /* short event flash */
  } else if (_seqCount && now - _seqStart < (uint32_t)_seqCount * 230u) {
    c = scale(_seqColor, blink(now - _seqStart, 230, 80));  /* self test OK: in time with the beeps */
  } else if (_booting) {
    c = scale(C_BLUE, breathe(now, 700));                /* start-up */
  } else if (_state == GUNTHER_ASCENT) {
    c = scale(C_BLUE, blink(now, 200, 100));
  } else if (_state == GUNTHER_DESCENT) {
    c = scale(C_PURPLE, blink(now, 200, 100));
  } else if (_state == GUNTHER_LANDED) {
    uint32_t t = now % 2000;                             /* beacon: double flash every 2 s */
    c = (t < 60 || (t >= 200 && t < 260)) ? C_WHITE : 0;
  } else if (_ledError) {
    uint32_t period = (uint32_t)_ledError * 400u + 1500u; /* N red flashes then a pause */
    uint32_t t = now % period;
    c = (t < (uint32_t)_ledError * 400u && (t % 400u) < 150u) ? C_RED : 0;
  } else if (_vbatCache > 3.0f && _vbatCache < _lowBatThreshold) {
    c = scale(C_ORANGE, blink(now, 1000, 500));          /* low battery */
  } else if (_logOK) {
    c = scale(C_GREEN, breathe(now, 3000));              /* ready, recording */
  } else {
    c = scale(C_CYAN, breathe(now, 3000));               /* ready, not recording */
  }

  float k = (float)_ledBright / 255.0f;
  if (c == C_WHITE && (int32_t)(_flashUntil - now) <= 0) k = k * 1.5f;   /* beacon a bit brighter */
  c = scale(c, k);
  ledShow((uint8_t)(c >> 16), (uint8_t)(c >> 8), (uint8_t)c);
}
