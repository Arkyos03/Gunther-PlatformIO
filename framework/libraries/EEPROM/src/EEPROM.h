/*
 * Gunther.ino - EEPROM library (emulated in flash), Arduino compatible API
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 *
 * The SAMD51 has no real EEPROM: 4 KB of data are kept in the last 8 KB block
 * of the flash (0x000FE000, reserved by the linker script). A copy is kept in RAM.
 *
 *   #include <EEPROM.h>
 *   EEPROM.write(0, 42);            byte  b = EEPROM.read(0);
 *   EEPROM.put(10, 3.14f);          float f; EEPROM.get(10, f);
 *   EEPROM[20] = 7;                 EEPROM.update(0, 42);   (writes only if different)
 *
 * Each write is saved at once in flash (like on an Arduino Uno). The flash can be
 * erased about 100 000 times: to write many values in a row, do
 *   EEPROM.setCommitASAP(false);  ...writes...  EEPROM.commit();
 * Never write in a fast loop (e.g. at every loop() during a flight): use the SD card.
 * Bytes never written read 0xFF (255), like a new EEPROM.
 */
#ifndef GUNTHER_EEPROM_H
#define GUNTHER_EEPROM_H

#include <Arduino.h>

#define EEPROM_EMULATION_SIZE  4096
#define E2END                  (EEPROM_EMULATION_SIZE - 1)

class EEPROMClass;

/* EEPROM[i] reference */
struct EERef {
  EERef(int index) : index(index) {}
  uint8_t operator*() const;
  operator uint8_t() const { return **this; }
  EERef &operator=(uint8_t value);
  EERef &operator=(const EERef &ref) { return *this = *ref; }
  EERef &operator+=(uint8_t v) { return *this = **this + v; }
  EERef &operator-=(uint8_t v) { return *this = **this - v; }
  EERef &operator*=(uint8_t v) { return *this = **this * v; }
  EERef &operator/=(uint8_t v) { return *this = **this / v; }
  EERef &operator&=(uint8_t v) { return *this = **this & v; }
  EERef &operator|=(uint8_t v) { return *this = **this | v; }
  EERef &operator^=(uint8_t v) { return *this = **this ^ v; }
  EERef &operator++() { return *this += 1; }
  EERef &operator--() { return *this -= 1; }
  uint8_t operator++(int) { uint8_t r = **this; ++(*this); return r; }
  uint8_t operator--(int) { uint8_t r = **this; --(*this); return r; }
  EERef &update(uint8_t value);
  int index;
};

class EEPROMClass {
public:
  uint8_t read(int index);
  void    write(int index, uint8_t value);
  void    update(int index, uint8_t value);
  EERef   operator[](int index) { return EERef(index); }
  uint16_t length() { return EEPROM_EMULATION_SIZE; }

  template <typename T> T &get(int index, T &value)
  {
    load();
    uint8_t *p = (uint8_t *)&value;
    for (size_t i = 0; i < sizeof(T); i++) p[i] = valid(index + i) ? _cache[index + i] : 0xFF;
    return value;
  }

  template <typename T> const T &put(int index, const T &value)
  {
    load();
    const uint8_t *p = (const uint8_t *)&value;
    for (size_t i = 0; i < sizeof(T); i++) {
      if (valid(index + i) && _cache[index + i] != p[i]) {
        _cache[index + i] = p[i];
        _dirty = true;
      }
    }
    if (_commitASAP) commit();
    return value;
  }

  /* Saving control */
  void setCommitASAP(bool on) { _commitASAP = on; }  /* default true */
  bool getCommitASAP() { return _commitASAP; }
  void commit();                                      /* save the RAM copy to flash (if changed) */
  bool isValid() { return true; }

  /* Erase everything (all bytes = 0xFF) */
  void clear();

private:
  friend struct EERef;
  void load();
  static bool valid(int index) { return index >= 0 && index < EEPROM_EMULATION_SIZE; }
  uint8_t _cache[EEPROM_EMULATION_SIZE] __attribute__((aligned(4)));
  bool _loaded = false;
  bool _dirty = false;
  bool _commitASAP = true;
};

extern EEPROMClass EEPROM;

#endif
