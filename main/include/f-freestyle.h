#ifndef F_FREESTYLE_H
#define F_FREESTYLE_H

#include <stdbool.h>
#include <time.h>

// Function declarations
bool init_freestyle_client(void);
void cleanup_freestyle_client(void);
bool login_freestyle(void);
bool fetch_freestyle_glucose(void);

// Note: Freestyle now uses unified glucose_data from frixos.h
extern bool freestyle_client_initialized;

#endif // F_FREESTYLE_H
