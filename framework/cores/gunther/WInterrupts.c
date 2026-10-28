/*
 * Gunther.ino Arduino core - external interrupts (attachInterrupt)
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 *
 * Each EXTINT line (0..15) can serve one pin at a time.
 */
#include "gunther_internal.h"

static voidFuncPtr s_callbacks[16];
static bool s_eic_ready = false;

static void eic_enable(bool on)
{
  if (on) EIC->CTRLA.reg |= EIC_CTRLA_ENABLE;
  else    EIC->CTRLA.reg &= (uint8_t)~EIC_CTRLA_ENABLE;
  while (EIC->SYNCBUSY.reg & EIC_SYNCBUSY_ENABLE) { }
}

static void eic_init(void)
{
  MCLK->APBAMASK.reg |= MCLK_APBAMASK_EIC;
  gunther_gclk_enable(EIC_GCLK_ID, GUNTHER_GCLK_48M);
  EIC->CTRLA.reg = EIC_CTRLA_SWRST;
  while (EIC->SYNCBUSY.reg & EIC_SYNCBUSY_SWRST) { }
  EIC->CTRLA.reg = 0;            /* CKSEL = 0 : GCLK_EIC */
  for (int i = 0; i < 16; i++) {
    NVIC_SetPriority((IRQn_Type)(EIC_0_IRQn + i), 2);
  }
  eic_enable(true);
  s_eic_ready = true;
}

void attachInterrupt(uint32_t pin, voidFuncPtr callback, uint32_t mode)
{
  if (pin >= NUM_DIGITAL_PINS) return;
  uint32_t line = g_pins[pin].extint;
  if (line > 15) return;
  if (!s_eic_ready) eic_init();

  uint32_t sense;
  switch (mode) {
    case LOW:     sense = EIC_CONFIG_SENSE0_LOW_Val;  break;
    case HIGH:    sense = EIC_CONFIG_SENSE0_HIGH_Val; break;
    case CHANGE:  sense = EIC_CONFIG_SENSE0_BOTH_Val; break;
    case FALLING: sense = EIC_CONFIG_SENSE0_FALL_Val; break;
    case RISING:  sense = EIC_CONFIG_SENSE0_RISE_Val; break;
    default: return;
  }

  /* Keep the pull-up / pull-down chosen with pinMode() and route the pin to the EIC */
  PortGroup *g = &PORT->Group[g_pins[pin].port];
  g->PINCFG[g_pins[pin].pin].reg |= PORT_PINCFG_INEN;
  gunther_pin_set_mux(pin, MUX_A);

  s_callbacks[line] = callback;

  eic_enable(false);                                   /* CONFIG is enable-protected */
  uint32_t shift = (line & 7u) * 4u;
  uint32_t cfg = EIC->CONFIG[line >> 3].reg;
  cfg &= ~(0xFu << shift);
  cfg |= (sense | EIC_CONFIG_FILTEN0) << shift;    /* filter: rejects glitches on long wires */
  EIC->CONFIG[line >> 3].reg = cfg;
  eic_enable(true);

  EIC->INTFLAG.reg = 1u << line;
  EIC->INTENSET.reg = 1u << line;
  NVIC_ClearPendingIRQ((IRQn_Type)(EIC_0_IRQn + line));
  NVIC_EnableIRQ((IRQn_Type)(EIC_0_IRQn + line));
}

void detachInterrupt(uint32_t pin)
{
  if (pin >= NUM_DIGITAL_PINS) return;
  uint32_t line = g_pins[pin].extint;
  if (line > 15 || !s_eic_ready) return;

  EIC->INTENCLR.reg = 1u << line;
  NVIC_DisableIRQ((IRQn_Type)(EIC_0_IRQn + line));
  s_callbacks[line] = 0;

  eic_enable(false);
  uint32_t shift = (line & 7u) * 4u;
  EIC->CONFIG[line >> 3].reg &= ~(0xFu << shift);
  eic_enable(true);

  gunther_pin_set_gpio(pin);
}

static inline void eic_dispatch(uint32_t line)
{
  EIC->INTFLAG.reg = 1u << line;
  if (s_callbacks[line]) s_callbacks[line]();
}

void EIC_0_Handler(void)  { eic_dispatch(0); }
void EIC_1_Handler(void)  { eic_dispatch(1); }
void EIC_2_Handler(void)  { eic_dispatch(2); }
void EIC_3_Handler(void)  { eic_dispatch(3); }
void EIC_4_Handler(void)  { eic_dispatch(4); }
void EIC_5_Handler(void)  { eic_dispatch(5); }
void EIC_6_Handler(void)  { eic_dispatch(6); }
void EIC_7_Handler(void)  { eic_dispatch(7); }
void EIC_8_Handler(void)  { eic_dispatch(8); }
void EIC_9_Handler(void)  { eic_dispatch(9); }
void EIC_10_Handler(void) { eic_dispatch(10); }
void EIC_11_Handler(void) { eic_dispatch(11); }
void EIC_12_Handler(void) { eic_dispatch(12); }
void EIC_13_Handler(void) { eic_dispatch(13); }
void EIC_14_Handler(void) { eic_dispatch(14); }
void EIC_15_Handler(void) { eic_dispatch(15); }
