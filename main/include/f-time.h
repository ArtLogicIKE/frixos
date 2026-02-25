#ifndef F_TIME_H
#define F_TIME_H    // Guard to prevent multiple inclusion

#include "time.h"

int get_moon_index();
void get_current_time(int *hour, int *minute);
void sync_time_with_ntp(void);


#endif