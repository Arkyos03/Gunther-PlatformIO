/*
 * Gunther.ino - Servo library (hardware PWM on TCC1, 50 Hz)
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 *
 * Works on SERVO_1 .. SERVO_4. Same API as the classic Arduino Servo library:
 *   Servo trappe;
 *   trappe.attach(SERVO_1);
 *   trappe.write(90);              // angle in degrees
 *   trappe.writeMicroseconds(1500);
 */
#ifndef GUNTHER_SERVO_H
#define GUNTHER_SERVO_H

#include <Arduino.h>

#define MIN_PULSE_WIDTH       544
#define MAX_PULSE_WIDTH      2400
#define DEFAULT_PULSE_WIDTH  1500
#define INVALID_SERVO         255
#define MAX_SERVOS              4

class Servo {
public:
  Servo();
  /* returns the servo channel (0..3) or INVALID_SERVO if the pin is not a servo output */
  uint8_t attach(int pin);
  uint8_t attach(int pin, int min_us, int max_us);
  void detach();
  void write(int value);                 /* < 200: angle (0..180), otherwise microseconds */
  void writeMicroseconds(int us);
  int  read();                           /* last angle written */
  int  readMicroseconds();
  bool attached();

private:
  int8_t   _pin;
  uint8_t  _cc;
  uint16_t _min, _max, _us;
};

#endif
