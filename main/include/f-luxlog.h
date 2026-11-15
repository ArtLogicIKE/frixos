#ifndef F_LUXLOG_H
#define F_LUXLOG_H

#include <time.h>
#include "cJSON.h"

// Rolling 24 h light history: 144 ten-minute averages in RTC memory, kept across
// restarts (not power loss), for the brightness chart in the web UI.

// Add one ALS reading taken at wall-clock time now (call only once time is valid).
void luxlog_add(double lux, time_t now);

// Add "luxhist": {"step":600,"end":<epoch of the newest slot>,"v":[oldest..newest]}
// to root; v holds lux or null for slots without readings.
void luxlog_add_json(cJSON *root);

#endif
