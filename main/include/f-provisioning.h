/* f-provisioning.h */
#ifndef F_PROVISIONING_H
#define F_PROVISIONING_H

#include <stdbool.h>
#include "esp_err.h"

#include "f-display.h"
#include "f-wifi.h"

// WiFi Provisioning Functions
void provision_init(void);
bool connect_to_wifi(void);

extern char portal_address_str[]; // we'll use Google DNS as our captive portal - make sure portal_address is set to this IP
extern char portal_address_dns[];
extern uint8_t portal_address[4];

#endif // F_PROVISIONING_H