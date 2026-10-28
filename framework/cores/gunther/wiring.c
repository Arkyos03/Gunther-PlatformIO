/*
 * Gunther.ino Arduino core - time keeping, init, GPIO helpers
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#include "gunther_internal.h"

/* ======================================================================
 * Time
 * ==================================================================== */
#define SYSTICK_RELOAD   (F_CPU / 1000UL)

static volatile uint32_t s_ms = 0;

void (*volatile gunther_led_hook)(void) = 0;

void SysTick_Handler(void)
{
  s_ms++;
  if (gunther_led_hook && (s_ms % 20u) == 0) SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
}

/* Lowest priority: LED animations never delay the other interrupts */
void PendSV_Handler(void)
{
  void (*hook)(void) = gunther_led_hook;
  if (hook) hook();
}

uint32_t millis(void)
{
  return s_ms;
}

uint32_t micros(void)
{
  uint32_t primask = __get_PRIMASK();
  __disable_irq();
  uint32_t ms    = s_ms;
  uint32_t ticks = SysTick->VAL;
  uint32_t pend  = SCB->ICSR & SCB_ICSR_PENDSTSET_Msk;
  __set_PRIMASK(primask);

  /* SysTick reloaded but its interrupt is not served yet */
  if (pend && ticks > (SYSTICK_RELOAD / 2)) ms++;

  return ms * 1000UL + (SYSTICK_RELOAD - 1UL - ticks) / (F_CPU / 1000000UL);
}

__attribute__((weak)) void yield(void)
{
  gunther_usb_task();
}

void delay(uint32_t ms)
{
  if (ms == 0) return;
  uint32_t start = micros();
  while (ms > 0) {
    yield();
    while (ms > 0 && (micros() - start) >= 1000UL) {
      ms--;
      start += 1000UL;
    }
  }
}

/* Cycle-accurate busy wait using the DWT cycle counter */
void delayMicroseconds(uint32_t us)
{
  if (us == 0) return;
  uint32_t start  = DWT->CYCCNT;
  uint32_t cycles = us * (F_CPU / 1000000UL);
  while ((DWT->CYCCNT - start) < cycles) { }
}

/* ======================================================================
 * Board start-up
 * ==================================================================== */
void init(void)
{
  SysTick_Config(SYSTICK_RELOAD);
  NVIC_SetPriority(SysTick_IRQn, 0);   /* time base has the highest priority */
  NVIC_SetPriority(PendSV_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);

  /* Cycle counter for delayMicroseconds() */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;

  /* Make sure the main peripheral buses are clocked */
  MCLK->APBAMASK.reg |= MCLK_APBAMASK_EIC;
  MCLK->APBBMASK.reg |= MCLK_APBBMASK_PORT;

  gunther_analog_init();
}

/* ======================================================================
 * Pin multiplexing helpers
 * ==================================================================== */
void gunther_port_set_mux(uint32_t port, uint32_t bit, uint32_t mux)
{
  PortGroup *g = &PORT->Group[port];
  if (bit & 1u) {
    g->PMUX[bit >> 1].reg = (g->PMUX[bit >> 1].reg & PORT_PMUX_PMUXE_Msk) | PORT_PMUX_PMUXO(mux);
  } else {
    g->PMUX[bit >> 1].reg = (g->PMUX[bit >> 1].reg & PORT_PMUX_PMUXO_Msk) | PORT_PMUX_PMUXE(mux);
  }
  g->PINCFG[bit].reg |= PORT_PINCFG_PMUXEN;
}

void gunther_pin_set_mux(uint32_t pin, uint32_t mux)
{
  if (pin >= NUM_DIGITAL_PINS) return;
  gunther_port_set_mux(g_pins[pin].port, g_pins[pin].pin, mux);
}

void gunther_pin_set_gpio(uint32_t pin)
{
  if (pin >= NUM_DIGITAL_PINS) return;
  PORT->Group[g_pins[pin].port].PINCFG[g_pins[pin].pin].reg &= (uint8_t)~PORT_PINCFG_PMUXEN;
}

/* ======================================================================
 * SERCOM helpers
 * ==================================================================== */
Sercom *gunther_sercom(uint32_t index)
{
  static Sercom *const list[] = { SERCOM0, SERCOM1, SERCOM2, SERCOM3, SERCOM4, SERCOM5 };
  return (index < 6) ? list[index] : 0;
}

void gunther_sercom_enable_clock(uint32_t index)
{
  static const uint8_t core_id[] = { SERCOM0_GCLK_ID_CORE, SERCOM1_GCLK_ID_CORE, SERCOM2_GCLK_ID_CORE,
                                     SERCOM3_GCLK_ID_CORE, SERCOM4_GCLK_ID_CORE, SERCOM5_GCLK_ID_CORE };
  switch (index) {
    case 0: MCLK->APBAMASK.reg |= MCLK_APBAMASK_SERCOM0; break;
    case 1: MCLK->APBAMASK.reg |= MCLK_APBAMASK_SERCOM1; break;
    case 2: MCLK->APBBMASK.reg |= MCLK_APBBMASK_SERCOM2; break;
    case 3: MCLK->APBBMASK.reg |= MCLK_APBBMASK_SERCOM3; break;
    case 4: MCLK->APBDMASK.reg |= MCLK_APBDMASK_SERCOM4; break;
    case 5: MCLK->APBDMASK.reg |= MCLK_APBDMASK_SERCOM5; break;
    default: return;
  }
  gunther_gclk_enable(core_id[index], GUNTHER_GCLK_48M);
  gunther_gclk_enable(SERCOM0_GCLK_ID_SLOW, GUNTHER_GCLK_32K);
}

uint32_t gunther_sercom_irq_base(uint32_t index)
{
  return (uint32_t)SERCOM0_0_IRQn + 4u * index;
}

/* ======================================================================
 * Bootloader
 * ==================================================================== */
#define UF2_DOUBLE_TAP_MAGIC   0xF01669EFUL
#define UF2_DOUBLE_TAP_ADDR    ((volatile uint32_t *)(HSRAM_ADDR + HSRAM_SIZE - 4))

void resetToBootloader(void)
{
  *UF2_DOUBLE_TAP_ADDR = UF2_DOUBLE_TAP_MAGIC;
  NVIC_SystemReset();
}

/* ======================================================================
 * Battery
 * ==================================================================== */
float readBatteryVoltage(void)
{
  uint32_t sum = 0;
  for (int i = 0; i < 8; i++) sum += (uint32_t)gunther_analog_read_raw(VBAT_SENSE);
  return ((float)sum / 8.0f) * gunther_analog_ref_volts() / 4095.0f * VBAT_DIVIDER_RATIO;
}
