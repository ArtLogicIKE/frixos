#ifndef F_DEXCOM_H
#define F_DEXCOM_H

#include <stdbool.h>
#include <time.h>

// Function declarations
bool init_dexcom_client(void);
void cleanup_dexcom_client(void);
bool authenticate_dexcom_account(void);
bool login_dexcom(void);
bool fetch_dexcom_glucose(void);
time_t parse_dexcom_timestamp(const char *timestamp);

// Note: Dexcom now uses unified glucose_data from frixos.h
extern char dexcom_account_id[64];
extern char dexcom_session_id[64];
extern bool dexcom_client_initialized;

#endif // F_DEXCOM_H 