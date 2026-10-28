/*
 * Gunther.ino Arduino core - USB device (CDC serial) built on TinyUSB (MIT)
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 *
 * The Arduino IDE resets the board into the bootloader by opening the
 * serial port at 1200 baud then closing it ("1200 bps touch").
 */
#include "gunther_internal.h"
#include "tusb.h"

/* ----------------------------------------------------------------------
 * USB identifiers
 *   0x1209 / 0x0001 is the pid.codes *test* PID. Before distributing the
 *   board, ask for your own free PID:
 *     - https://pid.codes (free for open-source hardware), or
 *     - Microchip VID sub-licensing program (free for Microchip MCUs).
 * -------------------------------------------------------------------- */
#ifndef USB_VID
#define USB_VID 0x1209
#endif
#ifndef USB_PID
#define USB_PID 0x0001
#endif
#ifndef USB_MANUFACTURER
#define USB_MANUFACTURER "AIR ESIEA"
#endif
#ifndef USB_PRODUCT
#define USB_PRODUCT "Gunther.ino"
#endif

static bool s_usb_started = false;

/* ---------------------------------------------------------------- init */
void gunther_usb_init(void)
{
  MCLK->AHBMASK.reg  |= MCLK_AHBMASK_USB;
  MCLK->APBBMASK.reg |= MCLK_APBBMASK_USB;
  gunther_gclk_enable(USB_GCLK_ID, GUNTHER_GCLK_48M);

  /* PA24 = D-, PA25 = D+ : peripheral function H */
  gunther_port_set_mux(0, 24, MUX_H);
  gunther_port_set_mux(0, 25, MUX_H);

  NVIC_SetPriority(USB_0_IRQn, 1);
  NVIC_SetPriority(USB_1_IRQn, 1);
  NVIC_SetPriority(USB_2_IRQn, 1);
  NVIC_SetPriority(USB_3_IRQn, 1);

  tusb_init();
  s_usb_started = true;
}

void gunther_usb_task(void)
{
  if (s_usb_started) tud_task();
}

void USB_0_Handler(void) { tud_int_handler(0); }
void USB_1_Handler(void) { tud_int_handler(0); }
void USB_2_Handler(void) { tud_int_handler(0); }
void USB_3_Handler(void) { tud_int_handler(0); }

/* ---------------------------------------------------------- 1200 touch */
static uint32_t s_last_baud = 115200;

void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const *coding)
{
  (void)itf;
  s_last_baud = coding->bit_rate;
}

void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
  (void)itf; (void)rts;
  if (!dtr && s_last_baud == 1200) {
    resetToBootloader();
  }
}

/* --------------------------------------------------------- descriptors */
static const tusb_desc_device_t s_device = {
  .bLength            = sizeof(tusb_desc_device_t),
  .bDescriptorType    = TUSB_DESC_DEVICE,
  .bcdUSB             = 0x0200,
  .bDeviceClass       = TUSB_CLASS_MISC,          /* IAD: composite with CDC */
  .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
  .bDeviceProtocol    = MISC_PROTOCOL_IAD,
  .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
  .idVendor           = USB_VID,
  .idProduct          = USB_PID,
  .bcdDevice          = 0x0100,
  .iManufacturer      = 1,
  .iProduct           = 2,
  .iSerialNumber      = 3,
  .bNumConfigurations = 1
};

const uint8_t *tud_descriptor_device_cb(void)
{
  return (const uint8_t *)&s_device;
}

enum { ITF_CDC_CTRL = 0, ITF_CDC_DATA, ITF_COUNT };
#define EP_CDC_NOTIF  0x81
#define EP_CDC_OUT    0x02
#define EP_CDC_IN     0x82
#define CONFIG_LEN    (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN)

static const uint8_t s_config[] = {
  TUD_CONFIG_DESCRIPTOR(1, ITF_COUNT, 0, CONFIG_LEN, 0x00, 100),
  TUD_CDC_DESCRIPTOR(ITF_CDC_CTRL, 4, EP_CDC_NOTIF, 8, EP_CDC_OUT, EP_CDC_IN, 64),
};

const uint8_t *tud_descriptor_configuration_cb(uint8_t index)
{
  (void)index;
  return s_config;
}

/* Serial number = 128-bit unique ID of the chip */
static void serial_string(char *out)
{
  static const uint32_t addr[4] = { 0x008061FC, 0x00806010, 0x00806014, 0x00806018 };
  static const char hex[] = "0123456789ABCDEF";
  for (int w = 0; w < 4; w++) {
    uint32_t v = *(const volatile uint32_t *)addr[w];
    for (int n = 7; n >= 0; n--) *out++ = hex[(v >> (n * 4)) & 0xF];
  }
  *out = '\0';
}

const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
  (void)langid;
  static uint16_t desc[34];
  char serial[33];
  const char *str;
  uint8_t len;

  switch (index) {
    case 0:  desc[1] = 0x0409; len = 1; desc[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * len + 2)); return desc;
    case 1:  str = USB_MANUFACTURER; break;
    case 2:  str = USB_PRODUCT; break;
    case 3:  serial_string(serial); str = serial; break;
    case 4:  str = "Gunther.ino Serial"; break;
    default: return 0;
  }
  len = (uint8_t)strlen(str);
  if (len > 32) len = 32;
  for (uint8_t i = 0; i < len; i++) desc[1 + i] = (uint16_t)str[i];
  desc[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * len + 2));
  return desc;
}
