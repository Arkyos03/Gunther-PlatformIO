/*
 * Gunther.ino Arduino core - program entry point
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#include "gunther_internal.h"

/* Optional hook for board specific initialisation */
__attribute__((weak)) void initVariant(void) {}

/* Called after each loop() when data is waiting on Serial1 */
__attribute__((weak)) void serialEvent1(void) {}
__attribute__((weak)) void serialEvent(void) {}

int main(void)
{
  init();
  initVariant();
  gunther_usb_init();

  setup();

  for (;;) {
    loop();
    gunther_usb_task();
    if (Serial1 && Serial1.available()) serialEvent1();
    if (Serial.available()) serialEvent();
  }
  return 0;
}
