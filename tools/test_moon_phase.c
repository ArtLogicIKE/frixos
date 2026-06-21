/*
 * Host-side sanity check for the moon-phase calculation.
 *
 * Builds the firmware's moon.c directly (it has no ESP-IDF dependencies)
 * and prints, for a date range, the new accurate icon index alongside the
 * old simplified one, plus elongation and illumination.
 *
 * Build & run:
 *   gcc tools/test_moon_phase.c main/moon.c -I main/include -lm -o /tmp/moontest
 *   /tmp/moontest
 *
 * Reference (timeanddate.com, UTC): in 2026 the Last/Third Quarter falls on
 * 8 June. The old code switched to "Last Quarter" around 5 June; the new
 * code should stay "Waning Gibbous" until ~8 June.
 */
#include <stdio.h>
#include <string.h>
#include "moon.h"

static const char *NAMES[8] = {
    "New Moon", "Waxing Crescent", "First Quarter", "Waxing Gibbous",
    "Full Moon", "Waning Gibbous", "Last Quarter", "Waning Crescent"};

static int days_in_month(int y, int m) {
    static const int d[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m == 2 && ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)) return 29;
    return d[m - 1];
}

int main(void) {
    int y = 2026;
    int months[] = {5, 6};

    printf("%-12s | %-3s %-16s | %-3s %-16s | %7s | %5s\n",
           "Date", "new", "(accurate)", "old", "(8-segment)", "elong", "illum");
    printf("-------------+----------------------+----------------------+---------+------\n");

    for (size_t mi = 0; mi < sizeof(months) / sizeof(months[0]); mi++) {
        int m = months[mi];
        int dmax = days_in_month(y, m);
        int start = (m == 5) ? 20 : 1;
        int end = (m == 6) ? 30 : dmax;
        for (int d = start; d <= end; d++) {
            uint8_t ni = calculateMoonIndexAccurate(y, m, d);
            uint8_t oi = calculateMoonPhase(y, m, d);
            double e = moonElongationDeg(y, m, d);
            MoonData md = calculateMoonData(y, m, d);
            char marker = (ni != oi) ? '*' : ' ';
            printf("%04d-%02d-%02d %c  | %2d  %-16s | %2d  %-16s | %6.1f  | %4.0f%%\n",
                   y, m, d, marker, ni, NAMES[ni], oi, NAMES[oi & 7], e, md.illumination * 100.0);
        }
    }
    printf("\n('*' = the two algorithms disagree for that day)\n");
    return 0;
}
