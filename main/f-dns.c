#include "f-dns.h"
#include "frixos.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <string.h>
#include <stdlib.h>

#define TAG "DnsServer"
#define DNS_SERVER_TASK_STACK_SIZE 4096
#define DNS_SERVER_TASK_PRIORITY 5
#define DNS_MAX_NAME_LENGTH 256

struct dns_server_t {
    int port;
    int socket_fd;
    esp_ip4_addr_t gateway;
    TaskHandle_t task_handle;
    bool is_running;
};

/**
 * @brief DNS server task function
 */
static void dns_server_task(void* arg);

/**
 * @brief Extract domain name from DNS query
 * 
 * @param dns_data Pointer to the start of the DNS query data
 * @param length Length of the DNS query data
 * @param name Buffer to store the extracted name
 * @param name_len Size of the name buffer
 * @return int Length of the extracted name or -1 on error
 */
static int extract_dns_name(const uint8_t* dns_data, int length, char* name, int name_len);

dns_server_t* dns_server_create(void) {
    dns_server_t* server = calloc(1, sizeof(dns_server_t));
    if (!server) {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to allocate memory for DNS server");
        return NULL;
    }
    
    server->port = 53;
    server->socket_fd = -1;
    server->is_running = false;
    
    return server;
}

void dns_server_free(dns_server_t* server) {
    if (server == NULL) {
        return;
    }
    
    if (server->is_running) {
        dns_server_stop(server);
    }
    
    free(server);
}

bool dns_server_start(dns_server_t* server, esp_ip4_addr_t gateway) {
    if (server == NULL || server->is_running) {
        return false;
    }
    
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Starting DNS server");
    server->gateway = gateway;

    server->socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server->socket_fd < 0) {
        ESP_LOG_WEB(ESP_LOG_ERROR,TAG, "Failed to create socket");
        return false;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(server->port);

    if (bind(server->socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to bind to port %d", server->port);
        close(server->socket_fd);
        server->socket_fd = -1;
        return false;
    }

    BaseType_t task_created = xTaskCreatePinnedToCore(
        dns_server_task,
        "DnsServerTask",
        DNS_SERVER_TASK_STACK_SIZE,
        server,
        DNS_SERVER_TASK_PRIORITY,
        &server->task_handle,
        0 // ping to the APP core
    );
    
    if (task_created != pdPASS) {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to create DNS server task");
        close(server->socket_fd);
        server->socket_fd = -1;
        return false;
    }
    
    server->is_running = true;
    return true;
}

void dns_server_stop(dns_server_t* server) {
    if (server == NULL || !server->is_running) {
        return;
    }
    
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Stopping DNS server");
    
    if (server->task_handle != NULL) {
        vTaskDelete(server->task_handle);
        server->task_handle = NULL;
    }
    
    if (server->socket_fd >= 0) {
        close(server->socket_fd);
        server->socket_fd = -1;
    }
    
    server->is_running = false;
}

static int extract_dns_name(const uint8_t* dns_data, int length, char* name, int name_len) {
    if (!dns_data || !name || name_len <= 0) {
        return -1;
    }
    
    // DNS header is 12 bytes, then the query starts
    const uint8_t* query = dns_data + 12;
    int pos = 0;
    int i = 0;
    
    // Extract the name from the query
    while (i < length - 12) {
        uint8_t len = query[i++];
        if (len == 0) {
            break; // End of domain name
        }
        
        // Check if we have enough space in the output buffer
        if (pos + len + 1 >= name_len) {
            return -1; // Not enough space
        }
        
        // Add a dot if this isn't the first label
        if (pos > 0) {
            name[pos++] = '.';
        }
        
        // Copy the label
        memcpy(name + pos, query + i, len);
        pos += len;
        i += len;
    }
    
    // Null-terminate the string
    if (pos < name_len) {
        name[pos] = '\0';
        return pos;
    }
    
    return -1; // Not enough space
}

static void dns_server_task(void* arg) {
    dns_server_t* server = (dns_server_t*)arg;
    uint8_t buffer[512];
    char domain_name[DNS_MAX_NAME_LENGTH];
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int len = recvfrom(server->socket_fd, buffer, sizeof(buffer), 0, 
                          (struct sockaddr*)&client_addr, &client_addr_len);
                          
        if (len < 0) {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "recvfrom failed, errno=%d", errno);
            continue;
        }

        // Extract and print the domain name
        if (extract_dns_name(buffer, len, domain_name, DNS_MAX_NAME_LENGTH) > 0) {
            ESP_LOG_WEB(ESP_LOG_VERBOSE,TAG, "Received DNS query for: %s", domain_name);
        } else {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Failed to extract domain name from DNS query");
        }

        // Simple DNS response: point all queries to gateway IP
        buffer[2] |= 0x80;  // Set response flag
        buffer[3] |= 0x80;  // Set Recursion Available
        buffer[7] = 1;      // Set answer count to 1

        // Add answer section
        memcpy(&buffer[len], "\xc0\x0c", 2);  // Name pointer
        len += 2;
        memcpy(&buffer[len], "\x00\x01\x00\x01\x00\x00\x00\x1c\x00\x04", 10);  // Type, class, TTL, data length
        len += 10;
        memcpy(&buffer[len], &server->gateway.addr, 4);  // Copy gateway IP
        len += 4;
        
        char ip_str[16];
        inet_ntop(AF_INET, &server->gateway.addr, ip_str, sizeof(ip_str));
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Sending DNS response for %s to %s", domain_name, ip_str);

        sendto(server->socket_fd, buffer, len, 0, (struct sockaddr*)&client_addr, client_addr_len);
    }
}