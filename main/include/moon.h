#ifndef MOON_H
#define MOON_H
#include <stdio.h>
#include <time.h>
#include <math.h>

typedef struct MoonData {
  uint8_t phase;
  double illumination;
} MoonData;

uint8_t calculateMoonIndex(time_t timestamp);
uint8_t calculateMoonPhase(uint16_t year, uint8_t month, uint8_t day);
MoonData calculateMoonData(uint16_t year, uint8_t month, uint8_t day);

#endif


