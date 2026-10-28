/*
 * Gunther.ino Arduino core - startup code for ATSAMD51J20A
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 *
 * Vector table generated from the device vector layout (DeviceVectors in samd51j20a.h).
 * Every handler is a weak alias of Default_Handler and can be overridden.
 */
#include "sam.h"

extern uint32_t __etext;        /* end of .text, start of .data load address */
extern uint32_t __data_start__, __data_end__;
extern uint32_t __bss_start__, __bss_end__;
extern uint32_t __StackTop;

extern int main(void);
extern void SystemInit(void);
extern void __libc_init_array(void);

void Reset_Handler(void);
void Default_Handler(void);

void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void) __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PM_Handler(void) __attribute__((weak, alias("Default_Handler")));
void MCLK_Handler(void) __attribute__((weak, alias("Default_Handler")));
void OSCCTRL_0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void OSCCTRL_1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void OSCCTRL_2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void OSCCTRL_3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void OSCCTRL_4_Handler(void) __attribute__((weak, alias("Default_Handler")));
void OSC32KCTRL_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SUPC_0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SUPC_1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void WDT_Handler(void) __attribute__((weak, alias("Default_Handler")));
void RTC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void EIC_0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void EIC_1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void EIC_2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void EIC_3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void EIC_4_Handler(void) __attribute__((weak, alias("Default_Handler")));
void EIC_5_Handler(void) __attribute__((weak, alias("Default_Handler")));
void EIC_6_Handler(void) __attribute__((weak, alias("Default_Handler")));
void EIC_7_Handler(void) __attribute__((weak, alias("Default_Handler")));
void EIC_8_Handler(void) __attribute__((weak, alias("Default_Handler")));
void EIC_9_Handler(void) __attribute__((weak, alias("Default_Handler")));
void EIC_10_Handler(void) __attribute__((weak, alias("Default_Handler")));
void EIC_11_Handler(void) __attribute__((weak, alias("Default_Handler")));
void EIC_12_Handler(void) __attribute__((weak, alias("Default_Handler")));
void EIC_13_Handler(void) __attribute__((weak, alias("Default_Handler")));
void EIC_14_Handler(void) __attribute__((weak, alias("Default_Handler")));
void EIC_15_Handler(void) __attribute__((weak, alias("Default_Handler")));
void FREQM_Handler(void) __attribute__((weak, alias("Default_Handler")));
void NVMCTRL_0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void NVMCTRL_1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DMAC_0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DMAC_1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DMAC_2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DMAC_3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DMAC_4_Handler(void) __attribute__((weak, alias("Default_Handler")));
void EVSYS_0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void EVSYS_1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void EVSYS_2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void EVSYS_3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void EVSYS_4_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PAC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TAL_0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TAL_1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void RAMECC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM0_0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM0_1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM0_2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM0_3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM1_0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM1_1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM1_2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM1_3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM2_0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM2_1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM2_2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM2_3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM3_0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM3_1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM3_2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM3_3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM4_0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM4_1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM4_2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM4_3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM5_0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM5_1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM5_2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM5_3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void USB_0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void USB_1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void USB_2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void USB_3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TCC0_0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TCC0_1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TCC0_2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TCC0_3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TCC0_4_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TCC0_5_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TCC0_6_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TCC1_0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TCC1_1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TCC1_2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TCC1_3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TCC1_4_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TCC2_0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TCC2_1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TCC2_2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TCC2_3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TCC3_0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TCC3_1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TCC3_2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TCC4_0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TCC4_1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TCC4_2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TC0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TC1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TC2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TC3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TC4_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TC5_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PDEC_0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PDEC_1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PDEC_2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void ADC0_0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void ADC0_1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void ADC1_0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void ADC1_1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void AC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DAC_0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DAC_1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DAC_2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DAC_3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DAC_4_Handler(void) __attribute__((weak, alias("Default_Handler")));
void I2S_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PCC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void AES_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TRNG_Handler(void) __attribute__((weak, alias("Default_Handler")));
void ICM_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PUKCC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void QSPI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SDHC0_Handler(void) __attribute__((weak, alias("Default_Handler")));

__attribute__((section(".vectors"), used))
const void *const exception_table[] = {
  (void *)&__StackTop,
  (void *)Reset_Handler,
  (void *)NMI_Handler,
  (void *)HardFault_Handler,
  (void *)MemManage_Handler,
  (void *)BusFault_Handler,
  (void *)UsageFault_Handler,
  (void *)0, /* ReservedM9 */
  (void *)0, /* ReservedM8 */
  (void *)0, /* ReservedM7 */
  (void *)0, /* ReservedM6 */
  (void *)SVC_Handler,
  (void *)DebugMon_Handler,
  (void *)0, /* ReservedM3 */
  (void *)PendSV_Handler,
  (void *)SysTick_Handler,
  (void *)PM_Handler,
  (void *)MCLK_Handler,
  (void *)OSCCTRL_0_Handler,
  (void *)OSCCTRL_1_Handler,
  (void *)OSCCTRL_2_Handler,
  (void *)OSCCTRL_3_Handler,
  (void *)OSCCTRL_4_Handler,
  (void *)OSC32KCTRL_Handler,
  (void *)SUPC_0_Handler,
  (void *)SUPC_1_Handler,
  (void *)WDT_Handler,
  (void *)RTC_Handler,
  (void *)EIC_0_Handler,
  (void *)EIC_1_Handler,
  (void *)EIC_2_Handler,
  (void *)EIC_3_Handler,
  (void *)EIC_4_Handler,
  (void *)EIC_5_Handler,
  (void *)EIC_6_Handler,
  (void *)EIC_7_Handler,
  (void *)EIC_8_Handler,
  (void *)EIC_9_Handler,
  (void *)EIC_10_Handler,
  (void *)EIC_11_Handler,
  (void *)EIC_12_Handler,
  (void *)EIC_13_Handler,
  (void *)EIC_14_Handler,
  (void *)EIC_15_Handler,
  (void *)FREQM_Handler,
  (void *)NVMCTRL_0_Handler,
  (void *)NVMCTRL_1_Handler,
  (void *)DMAC_0_Handler,
  (void *)DMAC_1_Handler,
  (void *)DMAC_2_Handler,
  (void *)DMAC_3_Handler,
  (void *)DMAC_4_Handler,
  (void *)EVSYS_0_Handler,
  (void *)EVSYS_1_Handler,
  (void *)EVSYS_2_Handler,
  (void *)EVSYS_3_Handler,
  (void *)EVSYS_4_Handler,
  (void *)PAC_Handler,
  (void *)TAL_0_Handler,
  (void *)TAL_1_Handler,
  (void *)0, /* Reserved44 */
  (void *)RAMECC_Handler,
  (void *)SERCOM0_0_Handler,
  (void *)SERCOM0_1_Handler,
  (void *)SERCOM0_2_Handler,
  (void *)SERCOM0_3_Handler,
  (void *)SERCOM1_0_Handler,
  (void *)SERCOM1_1_Handler,
  (void *)SERCOM1_2_Handler,
  (void *)SERCOM1_3_Handler,
  (void *)SERCOM2_0_Handler,
  (void *)SERCOM2_1_Handler,
  (void *)SERCOM2_2_Handler,
  (void *)SERCOM2_3_Handler,
  (void *)SERCOM3_0_Handler,
  (void *)SERCOM3_1_Handler,
  (void *)SERCOM3_2_Handler,
  (void *)SERCOM3_3_Handler,
  (void *)SERCOM4_0_Handler,
  (void *)SERCOM4_1_Handler,
  (void *)SERCOM4_2_Handler,
  (void *)SERCOM4_3_Handler,
  (void *)SERCOM5_0_Handler,
  (void *)SERCOM5_1_Handler,
  (void *)SERCOM5_2_Handler,
  (void *)SERCOM5_3_Handler,
  (void *)0, /* Reserved70 */
  (void *)0, /* Reserved71 */
  (void *)0, /* Reserved72 */
  (void *)0, /* Reserved73 */
  (void *)0, /* Reserved74 */
  (void *)0, /* Reserved75 */
  (void *)0, /* Reserved76 */
  (void *)0, /* Reserved77 */
  (void *)0, /* Reserved78 */
  (void *)0, /* Reserved79 */
  (void *)USB_0_Handler,
  (void *)USB_1_Handler,
  (void *)USB_2_Handler,
  (void *)USB_3_Handler,
  (void *)0, /* Reserved84 */
  (void *)TCC0_0_Handler,
  (void *)TCC0_1_Handler,
  (void *)TCC0_2_Handler,
  (void *)TCC0_3_Handler,
  (void *)TCC0_4_Handler,
  (void *)TCC0_5_Handler,
  (void *)TCC0_6_Handler,
  (void *)TCC1_0_Handler,
  (void *)TCC1_1_Handler,
  (void *)TCC1_2_Handler,
  (void *)TCC1_3_Handler,
  (void *)TCC1_4_Handler,
  (void *)TCC2_0_Handler,
  (void *)TCC2_1_Handler,
  (void *)TCC2_2_Handler,
  (void *)TCC2_3_Handler,
  (void *)TCC3_0_Handler,
  (void *)TCC3_1_Handler,
  (void *)TCC3_2_Handler,
  (void *)TCC4_0_Handler,
  (void *)TCC4_1_Handler,
  (void *)TCC4_2_Handler,
  (void *)TC0_Handler,
  (void *)TC1_Handler,
  (void *)TC2_Handler,
  (void *)TC3_Handler,
  (void *)TC4_Handler,
  (void *)TC5_Handler,
  (void *)0, /* Reserved113 */
  (void *)0, /* Reserved114 */
  (void *)PDEC_0_Handler,
  (void *)PDEC_1_Handler,
  (void *)PDEC_2_Handler,
  (void *)ADC0_0_Handler,
  (void *)ADC0_1_Handler,
  (void *)ADC1_0_Handler,
  (void *)ADC1_1_Handler,
  (void *)AC_Handler,
  (void *)DAC_0_Handler,
  (void *)DAC_1_Handler,
  (void *)DAC_2_Handler,
  (void *)DAC_3_Handler,
  (void *)DAC_4_Handler,
  (void *)I2S_Handler,
  (void *)PCC_Handler,
  (void *)AES_Handler,
  (void *)TRNG_Handler,
  (void *)ICM_Handler,
  (void *)PUKCC_Handler,
  (void *)QSPI_Handler,
  (void *)SDHC0_Handler,
  (void *)0, /* Reserved136 */
};

void Reset_Handler(void)
{
  uint32_t *src = &__etext;
  uint32_t *dst = &__data_start__;
  if (src != dst) {
    while (dst < &__data_end__) *dst++ = *src++;
  }
  for (dst = &__bss_start__; dst < &__bss_end__; ) *dst++ = 0;

  /* Vector table relocation (application starts after the bootloader) */
  SCB->VTOR = ((uint32_t)exception_table & SCB_VTOR_TBLOFF_Msk);

  /* Enable the FPU (CP10/CP11 full access) */
  SCB->CPACR |= (0xFu << 20);
  __DSB();
  __ISB();

  SystemInit();
  __libc_init_array();
  main();
  while (1) { }
}

/* Unhandled interrupt or fault: stop here (visible with a debugger). */
void Default_Handler(void)
{
  while (1) { }
}
