/*
 * Gunther.ino - common library of all the Gunther.ino boards (simplified mode)
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 *
 *   #include <Gunther.h>
 *
 *   void setup() {
 *     Gunther.begin();             // pins, sensors, ground altitude
 *     Gunther.selfTest();          // 3 beeps = everything OK
 *     Gunther.logBegin();          // new file vol_XXX.csv on the SD card
 *   }
 *   void loop() {
 *     Gunther.update();            // read the sensors, follow the flight
 *     Gunther.logFlight();         // one line of flight data
 *     if (Gunther.apogee()) Gunther.servoOpen(1);
 *   }
 *
 * Wiring on the terminal blocks:
 *   - switches between the pin and GND (internal pull-up: open = HIGH, closed = LOW)
 *   - launch jack between VCC and the pin (10k pull-down: plugged = HIGH, removed = LOW)
 *   - LEDs between the pin and GND (330 ohm resistor already on the board)
 */
#ifndef GUNTHER_LIB_H
#define GUNTHER_LIB_H

#include <Arduino.h>
#include "Gunther_Port.h"

/* Flight phases returned by Gunther.state() */
enum GuntherFlightState : uint8_t {
  GUNTHER_GROUND  = 0,   /* on the launch pad           */
  GUNTHER_ASCENT  = 1,   /* liftoff detected            */
  GUNTHER_DESCENT = 2,   /* apogee detected             */
  GUNTHER_LANDED  = 3,   /* back on the ground after the flight */
};

/* Error codes of Gunther.selfTest() (also given by the number of short beeps) */
enum GuntherError : uint8_t {
  GUNTHER_OK         = 0,
  GUNTHER_ERR_BARO   = 1,  /* barometer not found  */
  GUNTHER_ERR_IMU    = 2,  /* BMI088 not found     */
  GUNTHER_ERR_SD     = 3,  /* SD card missing / unreadable */
  GUNTHER_ERR_BATTERY = 4, /* battery too low      */
};

class GuntherBoard {
public:
  /* ------------------------------------------------------------- start-up */
  /* Configures the pins, starts the barometer and the IMU, measures the ground
   * altitude. Returns false if a sensor did not answer (the board still works). */
  bool begin();

  /* Reads the sensors and follows the flight (liftoff, apogee).
   * Call it as often as possible in loop(). The other functions also call it. */
  void update();

  /* Checks barometer, IMU, SD card and battery, prints a report on Serial and beeps:
   * 3 short beeps = OK, 1 long beep + N short beeps = error code N (GuntherError).
   * needBattery = false for tests on USB only. */
  GuntherError selfTest(bool needSdCard = true, bool needBattery = true);

  /* ------------------------------------------------------------- status LED
   * The built-in RGB LED shows what the board is doing (see the manual):
   *   blue breathing = start-up, green breathing = ready and recording,
   *   cyan breathing = ready without recording, red flashes = error code,
   *   orange blink = low battery, blue blink = ascent, purple blink = descent,
   *   white double flash = landed.
   * It stops as soon as the sketch uses the LED itself (Gunther.rgb(), rgbLedWrite,
   * digitalWrite(LED_BUILTIN...)), or it can be moved to an external LED. */
  void statusLed(bool on);                      /* automatic animations on / off (on by default, true = take the LED back) */
  void setStatusLed(uint8_t pin, bool addressable = true); /* external LED: WS2812 / SK6812 (true) or simple LED (false) */
  void statusBrightness(uint8_t level);         /* 0..255, default 60 */
  void statusFlash(uint32_t rgb, uint16_t ms = 200); /* short flash, e.g. Gunther.statusFlash(0x00FF00) */
  void rgb(uint8_t r, uint8_t g, uint8_t b);    /* built-in LED colour (stops the animations on it) */
  void rgb(uint32_t rgb);                       /* 0xRRGGBB */
  void _ledTask();                              /* internal: called every 20 ms */

  /* ------------------------------------------------------------- LEDs */
  void led(uint8_t n, bool on);                 /* n = 1..4 */
  void ledToggle(uint8_t n);
  void ledBrightness(uint8_t n, uint8_t level); /* 0..255, LED 1 and 2 only */
  void allLeds(bool on);

  /* ------------------------------------------------------------- inputs */
  bool switchClosed(uint8_t n);                 /* n = 1..4 */
  bool jackPlugged(uint8_t n = 1);

  /* ------------------------------------------------------------- buzzer */
  void beep(uint32_t frequency = 4000, uint32_t duration_ms = 100);
  void beepBlocking(uint32_t frequency = 4000, uint32_t duration_ms = 100);
  void beepCount(uint8_t count, uint32_t frequency = 4000);

  /* ------------------------------------------------------------- battery */
  float batteryVoltage();                       /* volts */
  bool  lowBattery(float threshold = 7.0f);     /* true below the threshold */

  /* ------------------------------------------------------------- barometer
   * BMP390, BMP388, BMP384 (Bosch) or SPL07-003 / SPL06 (sold as "BMP388" by
   * some Chinese makers, same footprint): detected automatically by begin(). */
  float pressure();                             /* Pa */
  float temperature();                          /* °C */
  float altitude();                             /* m, relative to the ground (setGroundLevel) */
  float altitudeSea(float seaLevel_hPa = 1013.25f); /* m, above sea level */
  void  setGroundLevel();                       /* altitude() = 0 here (done by begin) */
  float verticalSpeed();                        /* m/s, > 0 going up (filtered) */
  bool  baroOK() { return _baroOK; }
  const char *baroName();                       /* "BMP390", "BMP388", "SPL07"... */

  /* ------------------------------------------------------------- IMU (BMI088) */
  void  accel(float &x, float &y, float &z);    /* m/s² */
  void  gyro(float &x, float &y, float &z);     /* °/s  */
  float accelX(); float accelY(); float accelZ();
  float gyroX();  float gyroY();  float gyroZ();
  float gForce();                               /* total acceleration in g (1 at rest) */
  bool  imuOK() { return _imuOK; }

  /* ------------------------------------------------------------- flight */
  bool     liftoff();                           /* true once the rocket has left the pad */
  bool     apogee();                            /* true once the apogee has been detected */
  bool     landed();                            /* true once back on the ground */
  GuntherFlightState state() { update(); return _state; }
  uint32_t flightTime();                        /* ms since liftoff (0 on the ground) */
  float    maxAltitude();                       /* m, highest altitude since liftoff */
  void     resetFlight();                       /* back to GUNTHER_GROUND (ground tests) */

  /* Settings (optional) */
  void setApogeeDrop(float meters)       { _apogeeDrop = meters; }   /* default 3 m   */
  void setApogeeTimeout(uint32_t ms)     { _apogeeTimeout = ms; }    /* default 0 = off */
  void setApogeeMinTime(uint32_t ms)     { _apogeeMinTime = ms; }    /* default 1000 ms */
  void setLiftoffAccel(float g)          { _liftoffG = g; }          /* default 0 = off */
  void setLandingTime(uint32_t ms)       { _landingTime = ms; }      /* default 3000 ms still */

  /* Beeps an altitude (default: the max altitude) digit by digit, to read it
   * after the flight: 437 m = 4 beeps, pause, 3 beeps, pause, 7 beeps; 0 = long beep. */
  void beepAltitude(float meters = -1.0f);

  /* ------------------------------------------------------------- servos */
  void servoAngles(uint8_t n, int closedAngle, int openAngle); /* default 0 / 90 */
  void servoOpen(uint8_t n);                    /* n = 1..4 */
  void servoClose(uint8_t n);
  void servoWrite(uint8_t n, int angle);
  void servoDetach(uint8_t n);                  /* no more signal: the servo is free */

  /* ------------------------------------------------------------- SD card */
  bool sdCardPresent();

  /* Opens a new file vol_001.csv, vol_002.csv... (never overwrites a flight).
   * header: optional first line, e.g. "temps_ms;altitude". */
  bool logBegin(const char *prefix = "vol", const char *header = nullptr);
  bool logReady() { return _logOK; }
  const char *logFileName() { return _logName; }
  void setLogSeparator(char c) { _sep = c; }    /* default ';' */

  /* One line: time in ms then the values. Ex: Gunther.log(altitude, temperature); */
  template <typename... T> void log(T... values)
  {
    if (!_logOK) return;
    logStart();
    logValues(values...);
    logEnd();
  }
  /* One complete line: time, state, altitude, pressure, temperature, accel, gyro, battery */
  void logFlight();
  void logText(const char *text);               /* free text line */
  void logFlush();                              /* force the write to the card */
  void logClose();

  /* Same line as logFlight(), sent to a serial port (Serial, Serial1 for a radio...) */
  void printFlight(Print &out = Serial);
  void printFlightHeader(Print &out = Serial);

  /* ------------------------------------------------------------- misc */
  void enterBootloader() { gunther_port_bootloader(); }

  /* Raw access to the sensor buses for advanced users */
  uint8_t baroReadRegister(uint8_t reg);
  uint8_t accelReadRegister(uint8_t reg);
  uint8_t gyroReadRegister(uint8_t reg);

private:
  int  ledPin(uint8_t n);
  /* sensors */
  bool baroBegin();
  bool baroRead();
  bool baroPresent();
  bool splBegin();
  bool splRead();
  void speedUpdate();
  bool imuBegin();
  void imuRead();
  /* logging */
  bool sdTest();
  void logStart();
  void logEnd();
  void logValue(float v);
  void logValue(double v)        { logValue((float)v); }
  void logValue(int v);
  void logValue(long v)          { logValue((int)v); }
  void logValue(unsigned int v);
  void logValue(unsigned long v) { logValue((unsigned int)v); }
  void logValue(bool v)          { logValue((int)v); }
  void logValue(const char *v);
  void logValue(const String &v) { logValue(v.c_str()); }
  void logValues() {}
  template <typename F, typename... R> void logValues(F first, R... rest)
  {
    logValue(first);
    logValues(rest...);
  }
  void flightUpdate();
  void ledShow(uint8_t r, uint8_t g, uint8_t b);
  void ledStartTask();

  /* state */
  bool _begun = false;
  bool _baroOK = false, _imuOK = false;
  uint32_t _lastUpdate = 0;

  /* barometer */
  double _parT1, _parT2, _parT3;
  double _parP1, _parP2, _parP3, _parP4, _parP5, _parP6, _parP7, _parP8, _parP9, _parP10, _parP11;
  float _pressure = 101325.0f, _temperature = 20.0f, _groundPressure = 101325.0f;
  uint32_t _baroSamples = 0;
  uint8_t _baroType = 0;                        /* 0 none, 1 BMP390, 2 BMP388/384, 3 SPL07, 4 SPL06 */
  float _splC0, _splC1, _splC00, _splC10, _splC01, _splC11, _splC20, _splC21, _splC30, _splC31, _splC40;
  float _vSpeed = 0, _lastAlt = 0;
  uint32_t _lastAltTime = 0;

  /* IMU */
  float _ax = 0, _ay = 0, _az = 0, _gx = 0, _gy = 0, _gz = 0;

  /* flight */
  GuntherFlightState _state = GUNTHER_GROUND;
  uint32_t _liftoffTime = 0, _jackOutSince = 0, _highGSince = 0;
  float _maxAlt = 0;
  uint8_t _belowCount = 0;
  uint32_t _lastBaroSample = 0;
  float _apogeeDrop = 3.0f, _liftoffG = 0.0f;
  uint32_t _apogeeTimeout = 0, _apogeeMinTime = 1000;
  uint32_t _landingTime = 3000, _stillSince = 0;

  /* servos */
  int _servoClosed[4] = { 0, 0, 0, 0 };
  int _servoOpen[4]   = { 90, 90, 90, 90 };

  /* status LED */
  volatile bool _ledAuto = true;
  volatile bool _booting = false;
  uint8_t  _ledPin = gunther_port_builtin_led();
  bool     _ledTick = false;                    /* true: animated by the platform timer */
  uint32_t _lastLedTask = 0;
  bool     _ledAddressable = true;
  uint8_t  _ledBright = 60;
  volatile uint8_t _ledError = 0;               /* error code shown in red */
  volatile uint32_t _flashColor = 0, _flashUntil = 0;
  volatile uint8_t _seqCount = 0;               /* green flashes of the self test */
  volatile uint32_t _seqStart = 0, _seqColor = 0;
  float    _vbatCache = 0;
  uint32_t _vbatTime = 0;
  uint32_t _lastShown = 0xFFFFFFFF;
  float    _lowBatThreshold = 7.0f;

  /* log */
  bool _logOK = false;
  char _logName[16] = "";
  char _sep = ';';
  uint32_t _lastFlush = 0;
};

extern GuntherBoard Gunther;

#endif
