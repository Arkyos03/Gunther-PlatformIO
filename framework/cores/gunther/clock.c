/*
 * Gunther.ino Arduino core - clock configuration
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 *
 *   GCLK0 : 120 MHz  CPU            (DPLL0, reference = GCLK5)
 *   GCLK1 : 48 MHz   peripherals    (DFLL48M, USB clock recovery)
 *   GCLK3 : 32.768 kHz              (XOSC32K crystal, OSCULP32K as fallback)
 *   GCLK5 : 1 MHz    DPLL reference (DFLL48M / 48)
 *
 * The board works without the 32 kHz crystal: the DFLL is trimmed by the USB
 * start-of-frame when USB is connected (crystal-less USB).
 */
#include "sam.h"
#include <stdbool.h>

uint32_t SystemCoreClock = 48000000UL;
bool gunther_xosc32k_ok = false;

static void gclk_gen(uint32_t gen, uint32_t src, uint32_t div)
{
  GCLK->GENCTRL[gen].reg = GCLK_GENCTRL_SRC(src) | GCLK_GENCTRL_GENEN |
                           GCLK_GENCTRL_IDC | GCLK_GENCTRL_DIV(div);
  while (GCLK->SYNCBUSY.reg & (GCLK_SYNCBUSY_GENCTRL0 << gen)) { }
}

void SystemInit(void)
{
  /* Flash wait states follow the CPU frequency automatically */
  NVMCTRL->CTRLA.reg |= NVMCTRL_CTRLA_AUTOWS;

  /* --- 32.768 kHz crystal (do not block forever if it is missing) ------ */
  OSC32KCTRL->XOSC32K.reg = OSC32KCTRL_XOSC32K_ENABLE | OSC32KCTRL_XOSC32K_XTALEN |
                            OSC32KCTRL_XOSC32K_EN32K | OSC32KCTRL_XOSC32K_EN1K |
                            OSC32KCTRL_XOSC32K_CGM_XT | OSC32KCTRL_XOSC32K_STARTUP(2);
  for (uint32_t t = 0; t < 2000000UL; t++) {
    if (OSC32KCTRL->STATUS.reg & OSC32KCTRL_STATUS_XOSC32KRDY) { gunther_xosc32k_ok = true; break; }
  }

  /* --- Reset the clock generators ------------------------------------- */
  GCLK->CTRLA.reg = GCLK_CTRLA_SWRST;
  while (GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_SWRST) { }

  /* CPU temporarily on the 32 kHz ULP oscillator while the DFLL is reconfigured */
  gclk_gen(0, GCLK_GENCTRL_SRC_OSCULP32K_Val, 1);

  /* 32 kHz generator */
  gclk_gen(3, gunther_xosc32k_ok ? GCLK_GENCTRL_SRC_XOSC32K_Val : GCLK_GENCTRL_SRC_OSCULP32K_Val, 1);

  /* --- DFLL48M, open loop + USB clock recovery ------------------------- */
  OSCCTRL->DFLLCTRLA.reg = 0;
  OSCCTRL->DFLLMUL.reg = OSCCTRL_DFLLMUL_CSTEP(1) | OSCCTRL_DFLLMUL_FSTEP(1) |
                         OSCCTRL_DFLLMUL_MUL(48000);     /* 1 kHz USB SOF x 48000 */
  while (OSCCTRL->DFLLSYNC.reg & OSCCTRL_DFLLSYNC_DFLLMUL) { }
  OSCCTRL->DFLLCTRLB.reg = 0;
  while (OSCCTRL->DFLLSYNC.reg & OSCCTRL_DFLLSYNC_DFLLCTRLB) { }
  OSCCTRL->DFLLCTRLA.reg = OSCCTRL_DFLLCTRLA_ENABLE;
  while (OSCCTRL->DFLLSYNC.reg & OSCCTRL_DFLLSYNC_ENABLE) { }
  OSCCTRL->DFLLVAL.reg = OSCCTRL->DFLLVAL.reg;            /* load factory calibration */
  while (OSCCTRL->DFLLSYNC.reg & OSCCTRL_DFLLSYNC_DFLLVAL) { }
  OSCCTRL->DFLLCTRLB.reg = OSCCTRL_DFLLCTRLB_WAITLOCK | OSCCTRL_DFLLCTRLB_CCDIS |
                           OSCCTRL_DFLLCTRLB_USBCRM;
  while ((OSCCTRL->STATUS.reg & OSCCTRL_STATUS_DFLLRDY) == 0) { }

  /* 48 MHz peripheral generator and 1 MHz DPLL reference */
  gclk_gen(1, GCLK_GENCTRL_SRC_DFLL_Val, 1);
  gclk_gen(5, GCLK_GENCTRL_SRC_DFLL_Val, 48);

  /* --- DPLL0 = 1 MHz x 120 = 120 MHz ----------------------------------- */
  OSCCTRL->Dpll[0].DPLLCTRLA.reg = 0;
  while (OSCCTRL->Dpll[0].DPLLSYNCBUSY.reg & OSCCTRL_DPLLSYNCBUSY_ENABLE) { }

  GCLK->PCHCTRL[OSCCTRL_GCLK_ID_FDPLL0].reg = GCLK_PCHCTRL_GEN(5) | GCLK_PCHCTRL_CHEN;
  while ((GCLK->PCHCTRL[OSCCTRL_GCLK_ID_FDPLL0].reg & GCLK_PCHCTRL_CHEN) == 0) { }

  OSCCTRL->Dpll[0].DPLLRATIO.reg = OSCCTRL_DPLLRATIO_LDRFRAC(0) | OSCCTRL_DPLLRATIO_LDR(119);
  while (OSCCTRL->Dpll[0].DPLLSYNCBUSY.reg & OSCCTRL_DPLLSYNCBUSY_DPLLRATIO) { }
  OSCCTRL->Dpll[0].DPLLCTRLB.reg = OSCCTRL_DPLLCTRLB_REFCLK_GCLK | OSCCTRL_DPLLCTRLB_LBYPASS;
  OSCCTRL->Dpll[0].DPLLCTRLA.reg = OSCCTRL_DPLLCTRLA_ENABLE;
  while (OSCCTRL->Dpll[0].DPLLSYNCBUSY.reg & OSCCTRL_DPLLSYNCBUSY_ENABLE) { }
  while ((OSCCTRL->Dpll[0].DPLLSTATUS.reg & (OSCCTRL_DPLLSTATUS_CLKRDY | OSCCTRL_DPLLSTATUS_LOCK)) !=
         (OSCCTRL_DPLLSTATUS_CLKRDY | OSCCTRL_DPLLSTATUS_LOCK)) { }

  /* --- CPU on DPLL0 ---------------------------------------------------- */
  MCLK->CPUDIV.reg = MCLK_CPUDIV_DIV_DIV1;
  gclk_gen(0, GCLK_GENCTRL_SRC_DPLL0_Val, 1);
  SystemCoreClock = 120000000UL;

  /* Instruction/data cache */
  CMCC->CTRL.reg = CMCC_CTRL_CEN;
}
