#ifndef F_NIGHTSCOUT_H
#define F_NIGHTSCOUT_H

#include <stdbool.h>

bool init_nightscout_client(void);
void cleanup_nightscout_client(void);
bool fetch_nightscout_glucose(void);

extern bool nightscout_client_initialized;

#endif // F_NIGHTSCOUT_H
