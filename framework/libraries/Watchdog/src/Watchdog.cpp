/*
 * Gunther.ino - hardware watchdog (WDT clocked by the 1.024 kHz ultra low power oscillator)
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#include "Watchdog.h"

WatchdogClass Watchdog;

static void wdt_sync(uint32_t mask)
{
  while (WDT->SYNCBUSY.reg & mask) { }
}

uint32_t WatchdogClass::enable(uint32_t ms)
{
  /* PER = n : timeout = 8 << n cycles of 1.024 kHz  (n = 0..11) */
  uint32_t per = 0;
  while (per < 11 && ((8UL << per) * 1000UL) / 1024UL < ms) per++;

  disable();
  WDT->CONFIG.reg = WDT_CONFIG_PER(per);
  WDT->EWCTRL.reg = 0;
  WDT->CTRLA.reg = WDT_CTRLA_ENABLE;
  wdt_sync(WDT_SYNCBUSY_ENABLE);
  reset();
  return ((8UL << per) * 1000UL) / 1024UL;
}

void WatchdogClass::reset()
{
  if (!enabled()) return;
  wdt_sync(WDT_SYNCBUSY_CLEAR);
  WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;
}

void WatchdogClass::disable()
{
  WDT->CTRLA.reg = 0;
  wdt_sync(WDT_SYNCBUSY_ENABLE | WDT_SYNCBUSY_WEN | WDT_SYNCBUSY_ALWAYSON);
}

bool WatchdogClass::enabled()
{
  return (WDT->CTRLA.reg & WDT_CTRLA_ENABLE) != 0;
}

GuntherResetCause WatchdogClass::resetCause()
{
  uint8_t c = RSTC->RCAUSE.reg;
  if (c & RSTC_RCAUSE_WDT)                          return RESET_WATCHDOG;
  if (c & RSTC_RCAUSE_SYST)                         return RESET_SOFTWARE;
  if (c & RSTC_RCAUSE_EXT)                          return RESET_EXTERNAL;
  if (c & (RSTC_RCAUSE_BODCORE | RSTC_RCAUSE_BODVDD)) return RESET_BROWNOUT;
  if (c & RSTC_RCAUSE_POR)                          return RESET_POWER_ON;
  return RESET_OTHER;
}

const char *WatchdogClass::resetCauseText()
{
  switch (resetCause()) {
    case RESET_POWER_ON: return "mise sous tension";
    case RESET_BROWNOUT: return "tension trop basse";
    case RESET_EXTERNAL: return "bouton RESET";
    case RESET_WATCHDOG: return "watchdog (programme bloque)";
    case RESET_SOFTWARE: return "logiciel / televersement";
    default:             return "autre";
  }
}

void WatchdogClass::restart()
{
  NVIC_SystemReset();
}
