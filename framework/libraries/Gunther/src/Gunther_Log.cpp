/*
 * Gunther.ino - data logging on the micro SD card (uses SdFat, MIT license)
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#include "Gunther.h"
#include <SPI.h>
#include <SdFat.h>
#include "Gunther_Port.h"

static SdFat  s_sd;
static FsFile s_file;
static bool   s_sdStarted = false;

#define LOG_FLUSH_MS  500      /* data written to the card at least every 0.5 s */

static const char *DEFAULT_HEADER =
  "temps_ms;etat;altitude_m;pression_Pa;temperature_C;"
  "ax_ms2;ay_ms2;az_ms2;gx_dps;gy_dps;gz_dps;batterie_V";

static bool sdStart()
{
  if (s_sdStarted) return true;
  s_sdStarted = s_sd.begin(SdSpiConfig(SD_CS, DEDICATED_SPI, SD_SCK_MHZ(12), &GUNTHER_SD_SPI));
  return s_sdStarted;
}

bool GuntherBoard::sdTest()
{
  if (!sdCardPresent()) return false;
  return sdStart();
}

/* Card present? On Rev A the SD_DETECT pin does not work (see variant.h):
 * the card is detected by starting it, at most once per second. */
bool GuntherBoard::sdCardPresent()
{
#if SD_DETECT_WORKS
  return digitalRead(SD_DETECT) == HIGH;
#else
  static uint32_t lastProbe = 0;
  static bool present = false;
  if (_logOK) return true;                  /* file open: do not disturb the card */
  uint32_t now = millis();
  if (lastProbe != 0 && now - lastProbe < 1000) return present;
  lastProbe = now ? now : 1;
  s_sdStarted = false;                      /* card may have been changed: start it again */
  present = sdStart();
  return present;
#endif
}

bool GuntherBoard::logBegin(const char *prefix, const char *header)
{
  _logOK = false;
  if (!sdStart()) return false;

  /* first free name: vol_001.csv, vol_002.csv ... */
  for (int i = 1; i <= 999; i++) {
    snprintf(_logName, sizeof(_logName), "%.7s_%03d.csv", prefix, i);
    if (!s_sd.exists(_logName)) {
      if (!s_file.open(_logName, O_WRONLY | O_CREAT | O_EXCL)) return false;
      _logOK = true;
      break;
    }
  }
  if (!_logOK) return false;

  const char *h = header ? header : DEFAULT_HEADER;
  if (h[0]) {
    /* the default header uses ';' : adapt it to the chosen separator */
    for (const char *c = h; *c; c++) s_file.write((*c == ';') ? _sep : *c);
    s_file.write('\n');
  }
  s_file.sync();
  _lastFlush = millis();
  return true;
}

void GuntherBoard::logStart()
{
  s_file.print(millis());
}

void GuntherBoard::logEnd()
{
  s_file.write('\n');
  if (millis() - _lastFlush >= LOG_FLUSH_MS) logFlush();
}

void GuntherBoard::logValue(float v)          { s_file.write(_sep); s_file.print(v, 3); }
void GuntherBoard::logValue(int v)            { s_file.write(_sep); s_file.print(v); }
void GuntherBoard::logValue(unsigned int v)   { s_file.write(_sep); s_file.print(v); }
void GuntherBoard::logValue(const char *v)    { s_file.write(_sep); s_file.print(v); }

void GuntherBoard::logFlight()
{
  if (!_logOK) return;
  update();
  float alt = _baroOK ? 44330.0f * (1.0f - powf(_pressure / _groundPressure, 0.190295f)) : 0.0f;
  log((int)_state, alt, _pressure, _temperature, _ax, _ay, _az, _gx, _gy, _gz, batteryVoltage());
}

void GuntherBoard::printFlightHeader(Print &out)
{
  for (const char *c = DEFAULT_HEADER; *c; c++) out.write((*c == ';') ? _sep : *c);
  out.write('\r');
  out.write('\n');
}

void GuntherBoard::printFlight(Print &out)
{
  update();
  float alt = _baroOK ? 44330.0f * (1.0f - powf(_pressure / _groundPressure, 0.190295f)) : 0.0f;
  char line[160];
  snprintf(line, sizeof(line), "%lu%c%d%c%.3f%c%.3f%c%.3f%c%.3f%c%.3f%c%.3f%c%.3f%c%.3f%c%.3f%c%.3f\r\n",
           (unsigned long)millis(), _sep, (int)_state, _sep, alt, _sep, _pressure, _sep, _temperature, _sep,
           _ax, _sep, _ay, _sep, _az, _sep, _gx, _sep, _gy, _sep, _gz, _sep, batteryVoltage());
  out.print(line);
}

void GuntherBoard::logText(const char *text)
{
  if (!_logOK) return;
  logStart();
  logValue(text);
  logEnd();
}

void GuntherBoard::logFlush()
{
  if (!_logOK) return;
  s_file.sync();
  _lastFlush = millis();
}

void GuntherBoard::logClose()
{
  if (!_logOK) return;
  s_file.close();
  _logOK = false;
}
