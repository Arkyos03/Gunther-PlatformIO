/*
 * Gunther.ino Arduino core - USB CDC serial port
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#include "gunther_internal.h"
#include "tusb.h"

USBSerial Serial;

#define USB_WRITE_TIMEOUT_MS 100u

int USBSerial::available()
{
  gunther_usb_task();
  return (int)tud_cdc_available();
}

int USBSerial::peek()
{
  gunther_usb_task();
  uint8_t c;
  return tud_cdc_peek(&c) ? (int)c : -1;
}

int USBSerial::read()
{
  gunther_usb_task();
  if (!tud_cdc_available()) return -1;
  return (int)tud_cdc_read_char();
}

int USBSerial::availableForWrite()
{
  return (int)tud_cdc_write_available();
}

void USBSerial::flush()
{
  if (!tud_cdc_connected()) return;
  tud_cdc_write_flush();
  gunther_usb_task();
}

size_t USBSerial::write(const uint8_t *buffer, size_t size)
{
  if (!tud_cdc_connected()) {
    gunther_usb_task();
    return size;                      /* nobody listening: drop silently */
  }

  size_t sent = 0;
  uint32_t last_progress = millis();
  while (sent < size) {
    uint32_t n = tud_cdc_write(buffer + sent, (uint32_t)(size - sent));
    if (n) {
      sent += n;
      last_progress = millis();
    } else {
      tud_cdc_write_flush();
      gunther_usb_task();
      if (!tud_cdc_connected() || (millis() - last_progress) > USB_WRITE_TIMEOUT_MS) break;
    }
  }
  tud_cdc_write_flush();
  gunther_usb_task();
  return size;
}

USBSerial::operator bool()
{
  gunther_usb_task();
  return tud_cdc_connected();
}

bool USBSerial::dtr()
{
  return tud_cdc_connected();
}

uint32_t USBSerial::baud()
{
  cdc_line_coding_t coding;
  tud_cdc_get_line_coding(&coding);
  return coding.bit_rate;
}
