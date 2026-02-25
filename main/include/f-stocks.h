#ifndef F_STOCKS_H
#define F_STOCKS_H

#include "frixos.h"
#include "f-integrations.h"

// Function declarations
bool fetch_stock_quote(integration_token_t *token);
void parse_stock_entities(const char *input);

#endif // F_STOCKS_H 