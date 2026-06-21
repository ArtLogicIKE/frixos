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

/* Accurate Sun-Moon elongation in degrees (0=new, 90=first quarter,
 * 180=full, 270=last quarter), and the 0..7 phase icon index derived
 * from it with narrow bands around the four principal phases. */
double moonElongationDeg(uint16_t year, uint8_t month, uint8_t day);
uint8_t calculateMoonIndexAccurate(uint16_t year, uint8_t month, uint8_t day);

#endif


