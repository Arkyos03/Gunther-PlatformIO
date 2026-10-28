/*
 * Gunther.ino Arduino core - TinyUSB configuration (USB CDC serial port)
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#ifndef GUNTHER_TUSB_CONFIG_H
#define GUNTHER_TUSB_CONFIG_H

#define CFG_TUSB_MCU               OPT_MCU_SAMD51
#define CFG_TUSB_OS                OPT_OS_NONE
#define CFG_TUSB_RHPORT0_MODE      (OPT_MODE_DEVICE | OPT_MODE_FULL_SPEED)
#define CFG_TUSB_DEBUG             0

#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN         __attribute__((aligned(4)))

#define CFG_TUD_ENDPOINT0_SIZE     64

#define CFG_TUD_CDC                1
#define CFG_TUD_MSC                0
#define CFG_TUD_HID                0
#define CFG_TUD_MIDI               0
#define CFG_TUD_VENDOR             0

#define CFG_TUD_CDC_RX_BUFSIZE     512
#define CFG_TUD_CDC_TX_BUFSIZE     1024
#define CFG_TUD_CDC_EP_BUFSIZE     64

#endif
