/*
 * Gunther.ino - easy functions for the board
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#include "Gunther.h"
#include <Servo.h>

GuntherBoard Gunther;

static const uint8_t s_leds[4]     = { LED_1, LED_2, LED_3, LED_4 };
static const uint8_t s_switches[4] = { SWITCH_1, SWITCH_2, SWITCH_3, SWITCH_4 };
static const uint8_t s_servoPins[4] = { SERVO_1, SERVO_2, SERVO_3, SERVO_4 };
static Servo s_servos[4];

/* ======================================================================
 * Start-up
 * ==================================================================== */
bool GuntherBoard::begin()
{
  _booting = true;
  ledStartTask();                           /* blue breathing during start-up */
  for (uint8_t i = 0; i < 4; i++) { pinMode(s_leds[i], OUTPUT); digitalWrite(s_leds[i], LOW); }
  for (uint8_t i = 0; i < 4; i++) pinMode(s_switches[i], INPUT_PULLUP);
  pinMode(JACK_1, INPUT_PULLDOWN);          /* 10k pull-down on the board as well */
#ifdef SD_DETECT
  pinMode(SD_DETECT, INPUT_PULLUP);
#endif
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  _baroOK = baroBegin();
  _imuOK  = imuBegin();
  if (_baroOK) setGroundLevel();
  if (_imuOK) imuRead();
  _begun = true;
  resetFlight();
  _vbatCache = gunther_port_battery();
  _vbatTime = millis();
  _ledError = !_baroOK ? GUNTHER_ERR_BARO : (!_imuOK ? GUNTHER_ERR_IMU : 0);
  _booting = false;
  return _baroOK && _imuOK;
}

void GuntherBoard::update()
{
  if (!_begun) return;
  uint32_t now = millis();
  if (now == _lastUpdate) return;          /* at most once per millisecond */
  _lastUpdate = now;

  if (!_ledTick && _ledAuto && now - _lastLedTask >= 20) {   /* platform without LED timer */
    _lastLedTask = now;
    _ledTask();
  }
  if (now - _vbatTime >= 500) {             /* battery for the status LED */
    _vbatTime = now;
    _vbatCache = gunther_port_battery();
  }
  if (_imuOK) imuRead();
  bool newBaro = _baroOK && baroRead();
  flightUpdate();
  if (newBaro && _state == GUNTHER_ASCENT) {
    /* apogee: altitude below the maximum for 5 samples in a row */
    float alt = 44330.0f * (1.0f - powf(_pressure / _groundPressure, 0.190295f));
    if (alt > _maxAlt) { _maxAlt = alt; _belowCount = 0; }
    else if (_maxAlt - alt >= _apogeeDrop) _belowCount++;
    else _belowCount = 0;
    if (_belowCount >= 5 && flightTime() >= _apogeeMinTime) {
      _state = GUNTHER_DESCENT;
      statusFlash(0xFFFFFF, 300);
    }
  }
}

/* ======================================================================
 * Flight phases
 * ==================================================================== */
void GuntherBoard::flightUpdate()
{
  uint32_t now = millis();

  if (_state == GUNTHER_GROUND) {
    bool launched = false;

    /* jack pulled out for at least 20 ms */
    if (!jackPlugged(1)) {
      if (_jackOutSince == 0) _jackOutSince = now ? now : 1;
      else if (now - _jackOutSince >= 20) launched = true;
    } else {
      _jackOutSince = 0;
    }

    /* optional: acceleration above the threshold for 50 ms */
    if (_liftoffG > 0.0f && _imuOK) {
      float g = sqrtf(_ax * _ax + _ay * _ay + _az * _az) / 9.80665f;
      if (g >= _liftoffG) {
        if (_highGSince == 0) _highGSince = now ? now : 1;
        else if (now - _highGSince >= 50) launched = true;
      } else {
        _highGSince = 0;
      }
    }

    if (launched) {
      _state = GUNTHER_ASCENT;
      _liftoffTime = now;
      statusFlash(0xFFFFFF, 300);
      _maxAlt = _baroOK ? 44330.0f * (1.0f - powf(_pressure / _groundPressure, 0.190295f)) : 0.0f;
      _belowCount = 0;
    }
  } else if (_state == GUNTHER_ASCENT) {
    /* safety timer: apogee forced after the given time */
    if (_apogeeTimeout > 0 && now - _liftoffTime >= _apogeeTimeout) {
      _state = GUNTHER_DESCENT;
      statusFlash(0xFFFFFF, 300);
    }
  } else if (_state == GUNTHER_DESCENT && _baroOK) {
    /* landing: vertical speed below 1 m/s during _landingTime */
    if (fabsf(_vSpeed) < 1.0f) {
      if (_stillSince == 0) _stillSince = now ? now : 1;
      else if (now - _stillSince >= _landingTime) _state = GUNTHER_LANDED;
    } else {
      _stillSince = 0;
    }
  }
}

bool GuntherBoard::liftoff()
{
  if (!_begun) return !jackPlugged(1);      /* begin() not called: simple jack reading */
  update();
  return _state != GUNTHER_GROUND;
}

bool GuntherBoard::apogee()
{
  update();
  return _state >= GUNTHER_DESCENT;
}

bool GuntherBoard::landed()
{
  update();
  return _state == GUNTHER_LANDED;
}

uint32_t GuntherBoard::flightTime()
{
  if (_state == GUNTHER_GROUND) return 0;
  return millis() - _liftoffTime;
}

float GuntherBoard::maxAltitude()
{
  update();
  return _maxAlt;
}

void GuntherBoard::resetFlight()
{
  _state = GUNTHER_GROUND;
  _liftoffTime = 0;
  _jackOutSince = 0;
  _highGSince = 0;
  _maxAlt = 0.0f;
  _belowCount = 0;
  _stillSince = 0;
}

/* ======================================================================
 * Self test
 * ==================================================================== */
GuntherError GuntherBoard::selfTest(bool needSdCard, bool needBattery)
{
  if (!_begun) begin();

  bool baro = _baroOK && baroPresent();
  bool imu  = _imuOK && accelReadRegister(0x00) == 0x1E && gyroReadRegister(0x00) == 0x0F;
  bool sd   = !needSdCard || _logOK || sdTest();
  float vbat = batteryVoltage();
  bool bat  = !needBattery || !lowBattery();

  GuntherError err = GUNTHER_OK;
  if (!baro) err = GUNTHER_ERR_BARO;
  else if (!imu) err = GUNTHER_ERR_IMU;
  else if (!sd) err = GUNTHER_ERR_SD;
  else if (!bat) err = GUNTHER_ERR_BATTERY;

  if (Serial) {
    Serial.println("=== Gunther.ino : auto-test ===");
    Serial.printf("Barometre        : %s (%s)\r\n", baro ? "OK" : "ERREUR", baroName());
    Serial.printf("Centrale BMI088  : %s\r\n", imu ? "OK" : "ERREUR");
    if (needSdCard) Serial.printf("Carte SD         : %s\r\n", sd ? "OK" : "ERREUR");
    Serial.printf("Pile             : %.2f V %s\r\n", vbat, bat ? "OK" : "FAIBLE");
    if (baro) Serial.printf("Altitude sol     : %.1f m (pression %.0f Pa)\r\n", altitudeSea(), _pressure);
    Serial.println(err == GUNTHER_OK ? "=> tout est OK" : "=> ERREUR");
  }

  _ledError = (uint8_t)err;
  if (err == GUNTHER_OK) {
    _seqColor = 0x00FF20;                   /* 3 green flashes with the 3 beeps */
    _seqStart = millis();
    _seqCount = 3;
    beepCount(3, 4000);
  } else {
    beepBlocking(2000, 1000);
    delay(400);
    beepCount((uint8_t)err, 3000);
  }
  return err;
}

/* ======================================================================
 * LEDs, inputs, buzzer, battery
 * ==================================================================== */
int GuntherBoard::ledPin(uint8_t n)
{
  return (n >= 1 && n <= 4) ? s_leds[n - 1] : -1;
}

void GuntherBoard::led(uint8_t n, bool on)
{
  int p = ledPin(n);
  if (p < 0) return;
  pinMode(p, OUTPUT);
  digitalWrite(p, on ? HIGH : LOW);
}

void GuntherBoard::ledToggle(uint8_t n)
{
  int p = ledPin(n);
  if (p >= 0) gunther_port_toggle(p);
}

void GuntherBoard::ledBrightness(uint8_t n, uint8_t level)
{
  int p = ledPin(n);
  if (p >= 0) analogWrite(p, level);
}

void GuntherBoard::allLeds(bool on)
{
  for (uint8_t n = 1; n <= 4; n++) led(n, on);
}

bool GuntherBoard::switchClosed(uint8_t n)
{
  if (n < 1 || n > 4) return false;
  return digitalRead(s_switches[n - 1]) == LOW;
}

bool GuntherBoard::jackPlugged(uint8_t n)
{
  if (n != 1) return false;
  return digitalRead(JACK_1) == HIGH;       /* jack plugged: pin tied to VCC */
}

void GuntherBoard::beep(uint32_t frequency, uint32_t duration_ms)
{
  tone(BUZZER, frequency, duration_ms);
}

void GuntherBoard::beepBlocking(uint32_t frequency, uint32_t duration_ms)
{
  tone(BUZZER, frequency, duration_ms);
  delay(duration_ms);
}

void GuntherBoard::beepCount(uint8_t count, uint32_t frequency)
{
  for (uint8_t i = 0; i < count; i++) {
    beepBlocking(frequency, 80);
    delay(150);
  }
}

void GuntherBoard::beepAltitude(float meters)
{
  if (meters < 0.0f) meters = _maxAlt;
  long m = lroundf(meters);
  if (m < 0) m = 0;
  char digits[12];
  snprintf(digits, sizeof(digits), "%ld", m);
  for (char *d = digits; *d; d++) {
    int n = *d - '0';
    if (n == 0) { statusFlash(0xFFFFFF, 600); beepBlocking(3000, 600); }   /* 0 = one long beep */
    for (int i = 0; i < n; i++) { statusFlash(0xFFFFFF, 120); beepBlocking(4000, 120); delay(250); }
    delay(1000);
  }
}

float GuntherBoard::batteryVoltage()
{
  return gunther_port_battery();
}

bool GuntherBoard::lowBattery(float threshold)
{
  return batteryVoltage() < threshold;
}

/* ======================================================================
 * Servos
 * ==================================================================== */
void GuntherBoard::servoAngles(uint8_t n, int closedAngle, int openAngle)
{
  if (n < 1 || n > 4) return;
  _servoClosed[n - 1] = closedAngle;
  _servoOpen[n - 1] = openAngle;
}

void GuntherBoard::servoWrite(uint8_t n, int angle)
{
  if (n < 1 || n > 4) return;
  Servo &s = s_servos[n - 1];
  if (!s.attached()) s.attach(s_servoPins[n - 1]);
  s.write(angle);
}

void GuntherBoard::servoDetach(uint8_t n)
{
  if (n >= 1 && n <= 4 && s_servos[n - 1].attached()) s_servos[n - 1].detach();
}

void GuntherBoard::servoOpen(uint8_t n)
{
  if (n >= 1 && n <= 4) servoWrite(n, _servoOpen[n - 1]);
}

void GuntherBoard::servoClose(uint8_t n)
{
  if (n >= 1 && n <= 4) servoWrite(n, _servoClosed[n - 1]);
}


