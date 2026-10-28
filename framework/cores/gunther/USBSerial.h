/*
 * Gunther.ino Arduino core - "Serial" = USB virtual serial port (CDC)
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 *
 * Serial.print() never blocks the program when no PC is connected:
 * the data is simply dropped (important during a flight!).
 */
#ifndef GUNTHER_USBSERIAL_H
#define GUNTHER_USBSERIAL_H

#include "HardwareSerial.h"

class USBSerial : public HardwareSerial {
public:
  void begin(unsigned long baud) override { (void)baud; }
  void begin(unsigned long baud, uint16_t config) override { (void)baud; (void)config; }
  void end() override {}

  int available() override;
  int peek() override;
  int read() override;
  int availableForWrite() override;
  void flush() override;
  size_t write(uint8_t c) override { return write(&c, 1); }
  size_t write(const uint8_t *buffer, size_t size) override;
  using Print::write;

  /* true when a terminal is open on the PC (DTR set) */
  operator bool() override;
  bool dtr();
  uint32_t baud();
};

extern USBSerial Serial;
#define SerialUSB Serial      /* name used on other SAMD boards */

#endif
