/*
 * Gunther.ino Arduino core - common base class of Serial (USB) and Serial1 (UART)
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#ifndef GUNTHER_HARDWARESERIAL_H
#define GUNTHER_HARDWARESERIAL_H

#include "Stream.h"

/* Frame format: data bits | parity | stop bits */
#define SERIAL_DATA_5   0x0005
#define SERIAL_DATA_6   0x0006
#define SERIAL_DATA_7   0x0007
#define SERIAL_DATA_8   0x0008
#define SERIAL_DATA_MASK 0x000F
#define SERIAL_PARITY_NONE 0x0000
#define SERIAL_PARITY_EVEN 0x0010
#define SERIAL_PARITY_ODD  0x0020
#define SERIAL_PARITY_MASK 0x00F0
#define SERIAL_STOP_1   0x0000
#define SERIAL_STOP_2   0x0100
#define SERIAL_STOP_MASK 0x0F00

#define SERIAL_8N1 (SERIAL_DATA_8 | SERIAL_PARITY_NONE | SERIAL_STOP_1)
#define SERIAL_8N2 (SERIAL_DATA_8 | SERIAL_PARITY_NONE | SERIAL_STOP_2)
#define SERIAL_8E1 (SERIAL_DATA_8 | SERIAL_PARITY_EVEN | SERIAL_STOP_1)
#define SERIAL_8E2 (SERIAL_DATA_8 | SERIAL_PARITY_EVEN | SERIAL_STOP_2)
#define SERIAL_8O1 (SERIAL_DATA_8 | SERIAL_PARITY_ODD  | SERIAL_STOP_1)
#define SERIAL_8O2 (SERIAL_DATA_8 | SERIAL_PARITY_ODD  | SERIAL_STOP_2)
#define SERIAL_7N1 (SERIAL_DATA_7 | SERIAL_PARITY_NONE | SERIAL_STOP_1)
#define SERIAL_7E1 (SERIAL_DATA_7 | SERIAL_PARITY_EVEN | SERIAL_STOP_1)
#define SERIAL_7O1 (SERIAL_DATA_7 | SERIAL_PARITY_ODD  | SERIAL_STOP_1)

class HardwareSerial : public Stream {
public:
  virtual void begin(unsigned long baud) = 0;
  virtual void begin(unsigned long baud, uint16_t config) = 0;
  virtual void end() = 0;
  virtual operator bool() = 0;
  using Print::write;
};

#endif
