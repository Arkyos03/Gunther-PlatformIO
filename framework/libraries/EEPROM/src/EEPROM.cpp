/*
 * Gunther.ino - EEPROM library (emulated in flash)
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#include "EEPROM.h"

EEPROMClass EEPROM;

#define EEPROM_FLASH_ADDR   0x000FE000UL    /* last 8 KB block of the 1 MB flash */
#define NVM_PAGE_SIZE       512u

uint8_t EERef::operator*() const    { return EEPROM.read(index); }
EERef &EERef::operator=(uint8_t v)  { EEPROM.write(index, v); return *this; }
EERef &EERef::update(uint8_t v)     { EEPROM.update(index, v); return *this; }

void EEPROMClass::load()
{
  if (_loaded) return;
  memcpy(_cache, (const void *)EEPROM_FLASH_ADDR, EEPROM_EMULATION_SIZE);
  _loaded = true;
  _dirty = false;
}

uint8_t EEPROMClass::read(int index)
{
  if (!valid(index)) return 0xFF;
  load();
  return _cache[index];
}

void EEPROMClass::write(int index, uint8_t value)
{
  if (!valid(index)) return;
  load();
  if (_cache[index] != value) {
    _cache[index] = value;
    _dirty = true;
  }
  if (_commitASAP) commit();
}

void EEPROMClass::update(int index, uint8_t value)
{
  write(index, value);     /* write() already skips identical values */
}

void EEPROMClass::clear()
{
  load();
  memset(_cache, 0xFF, sizeof(_cache));
  _dirty = true;
  commit();
}

static void nvm_wait(void)
{
  while ((NVMCTRL->STATUS.reg & NVMCTRL_STATUS_READY) == 0) { }
}

static void nvm_command(uint32_t cmd)
{
  nvm_wait();
  NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | cmd;
  nvm_wait();
  NVMCTRL->INTFLAG.reg = NVMCTRL_INTFLAG_MASK;   /* clear DONE and error flags */
}

void EEPROMClass::commit()
{
  if (!_loaded || !_dirty) return;

  /* manual write mode, NVM caches off during the operation */
  uint16_t ctrla = NVMCTRL->CTRLA.reg;
  NVMCTRL->CTRLA.reg = (uint16_t)((ctrla & ~NVMCTRL_CTRLA_WMODE_Msk) | NVMCTRL_CTRLA_WMODE_MAN |
                                  NVMCTRL_CTRLA_CACHEDIS0 | NVMCTRL_CTRLA_CACHEDIS1);

  NVMCTRL->ADDR.reg = EEPROM_FLASH_ADDR;
  nvm_command(NVMCTRL_CTRLB_CMD_EB);                 /* erase the 8 KB block */

  for (uint32_t off = 0; off < EEPROM_EMULATION_SIZE; off += NVM_PAGE_SIZE) {
    nvm_command(NVMCTRL_CTRLB_CMD_PBC);              /* clear the page buffer */
    volatile uint32_t *dst = (volatile uint32_t *)(EEPROM_FLASH_ADDR + off);
    const uint32_t *src = (const uint32_t *)(_cache + off);
    for (uint32_t i = 0; i < NVM_PAGE_SIZE / 4u; i++) dst[i] = src[i];
    NVMCTRL->ADDR.reg = EEPROM_FLASH_ADDR + off;
    nvm_command(NVMCTRL_CTRLB_CMD_WP);               /* write the page */
  }

  NVMCTRL->CTRLA.reg = ctrla;
  _dirty = false;
}
