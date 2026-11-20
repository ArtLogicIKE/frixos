#pragma once

#include <stdbool.h>
#include "esp_http_server.h"

// Optional access password for the web UI and API. The device stores only
// PBKDF2-SHA256(password, "frixos:" + STA MAC), computed by the client, and checks
// logins with a one-time challenge, so the password never crosses the network.

void auth_init(void);   // load the stored hash (call before serving requests)
bool auth_enabled(void);
void auth_erase(void);  // remove the password (rescue reset)

// Registered as every URI handler; user_ctx holds the real handler.
esp_err_t auth_gate(httpd_req_t *req);

esp_err_t auth_challenge_handler(httpd_req_t *req); // GET  /api/auth/challenge
esp_err_t auth_login_handler(httpd_req_t *req);     // POST /api/auth/login
esp_err_t auth_logout_handler(httpd_req_t *req);    // POST /api/auth/logout
esp_err_t auth_password_handler(httpd_req_t *req);  // POST /api/auth/password
