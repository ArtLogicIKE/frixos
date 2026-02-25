#ifndef F_DNS_H
#define F_DNS_H


#include <esp_netif_ip_addr.h>
#include <stdbool.h>

// Opaque structure for DNS server
typedef struct dns_server_t dns_server_t;

/**
 * @brief Create a new DNS server instance
 * 
 * @return Pointer to the DNS server instance or NULL if failed
 */
dns_server_t* dns_server_create(void);

/**
 * @brief Free a DNS server instance
 * 
 * @param server DNS server instance
 */
void dns_server_free(dns_server_t* server);

/**
 * @brief Start the DNS server
 * 
 * @param server DNS server instance
 * @param gateway Gateway IP address
 * @return true if started successfully, false otherwise
 */
bool dns_server_start(dns_server_t* server, esp_ip4_addr_t gateway);

/**
 * @brief Stop the DNS server
 * 
 * @param server DNS server instance
 */
void dns_server_stop(dns_server_t* server);

#endif // F_DNS_H
