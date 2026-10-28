/*
 * Gunther.ino - hardware watchdog
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 *
 * If the program freezes (infinite loop, sensor that never answers...), the
 * watchdog restarts the board after the chosen time.
 *
 *   #include <Watchdog.h>
 *   void setup() { Watchdog.enable(2000); }      // restart if blocked more than ~2 s
 *   void loop()  { Watchdog.reset(); ... }       // "I am alive": call it often
 *
 * Possible times: 8, 16, 31, 63, 125, 250, 500 ms, 1, 2, 4, 8, 16 s
 * (the next value above the one asked is used, enable() returns it in ms).
 */
#ifndef GUNTHER_WATCHDOG_H
#define GUNTHER_WATCHDOG_H

#include <Arduino.h>

/* Cause of the last restart, returned by Watchdog.resetCause() */
enum GuntherResetCause : uint8_t {
  RESET_POWER_ON  = 0,   /* power switched on                    */
  RESET_BROWNOUT  = 1,   /* supply voltage too low               */
  RESET_EXTERNAL  = 2,   /* RESET button                         */
  RESET_WATCHDOG  = 3,   /* watchdog: the program was blocked    */
  RESET_SOFTWARE  = 4,   /* NVIC_SystemReset(), upload           */
  RESET_OTHER     = 5,
};

class WatchdogClass {
public:
  uint32_t enable(uint32_t ms = 2000);   /* returns the real time in ms */
  void     reset();                      /* call it more often than the time */
  void     disable();
  bool     enabled();

  GuntherResetCause resetCause();
  const char   *resetCauseText();        /* "mise sous tension", "watchdog"... */
  void          restart();               /* immediate software restart */
};

extern WatchdogClass Watchdog;

#endif
