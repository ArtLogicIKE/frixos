#ifndef F_INTEGRATIONS_H
#define F_INTEGRATIONS_H

#include <stddef.h>
#include <time.h>
#include "esp_crt_bundle.h"
#include "cJSON.h"

#define AVAILABLE_INTEGRATIONS 5
#define INTEGRATION_HA    0  // Home Assistant integration index
#define INTEGRATION_STOCK 1  // Stock quote integration index
#define INTEGRATION_DEXCOM 2  // Dexcom integration index
#define INTEGRATION_FREESTYLE 3 // Freestyle Libre integration index
#define INTEGRATION_NIGHTSCOUT 4 // Nightscout integration index

// Stack monitoring
#define STACK_WARNING_THRESHOLD 512 // Warning when less than 512 bytes free
#define AUTH_BUFFER_SIZE 256        // Reduced from 512
#define URL_BUFFER_SIZE 512

// Define structure for HA tokens
typedef struct {    
        char *name;          // Token name as it appears in the message (e.g. "[HA:sensor.temperature:state]") - dynamically allocated
        char *entity;        // Home Assistant entity ID, stock symbol, etc. - dynamically allocated
        char *path;          // JSON path to extract value - dynamically allocated
        char value[64];      // Current value (will be updated by HA integration) - static buffer        
} integration_token_t;

// Global variables for integrations
extern integration_token_t *integration_active_tokens[AVAILABLE_INTEGRATIONS];  // Dynamic array of tokens for each integration
extern int integration_active_tokens_count[AVAILABLE_INTEGRATIONS];    // Number of active tokens for each integration
extern int integration_active[AVAILABLE_INTEGRATIONS];                 // Whether each integration is active
extern time_t integration_last_update[AVAILABLE_INTEGRATIONS];         // Last update time for each integration

// Function declarations
esp_err_t custom_crt_bundle_attach(void *conf);
void parse_HA_entities(const char *input);
void parse_stock_entities(const char *input);
void parse_integrations(void);
void startup_integrations(void);
void force_integration_update(void);
cJSON *get_nested_json_value(cJSON *json, const char *path);

// Token management functions
bool init_integration_token(integration_token_t *token, const char *name, const char *entity, const char *path);
void free_integration_token(integration_token_t *token);
void free_integration_tokens_array(integration_token_t *tokens, int count);

// SSL connection semaphore helpers (must be used before any SSL connection)
bool acquire_ssl_semaphore(const char *function_name);
void release_ssl_semaphore(void);

#endif // F_INTEGRATIONS_H 