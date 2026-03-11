#include "f-provisioning.h"
#include "frixos.h"
#include "config.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "lwip/ip_addr.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "mdns.h"
#include <string.h>

#include "f-dns.h"
#include "f-settings.h"
#include "f-display.h"

#define WIFI_TIMEOUT_SEC 60
#define AP_SSID "frixos"
#define AP_PASS ""
#define PROVISIONING_TIMEOUT_MINUTES 10
#define PROVISIONING_TIMEOUT_MS (PROVISIONING_TIMEOUT_MINUTES * 60 * 1000)

// Removed captive_uri declaration (Option 114)

static const char *TAG = "f-provisioning";
uint8_t portal_address[4] = {8, 8, 8, 8}; // {192,168,4,1}; // {8,8,8,8}; // we'll capture the IP of Google DNS for our portal
char portal_address_str[16] = "8.8.8.8";  // 192.168.4.1"; // we'll use Google DNS as our captive portal - make sure portal_address is set to this IP
char portal_address_dns[16] = "8.8.8.8";  //"192.168.4.1";


bool connect_to_wifi(void)
{
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    esp_netif_set_hostname(sta_netif, eeprom_hostname);
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    // First try stored credentials
    wifi_config_t wifi_config = {0};
    strncpy((char *)wifi_config.sta.ssid, eeprom_wifi_ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, eeprom_wifi_pass, sizeof(wifi_config.sta.password));

    if (strlen((char *)wifi_config.sta.ssid) > 0) // if we have stored credentials
    {
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Try stored SSID %s", (char *)wifi_config.sta.ssid);
        snprintf(msg_scrolling, SCROLL_MSG_LENGTH, "Connecting to %s", (char *)wifi_config.sta.ssid);
        lvgl_port_lock(0);
        set_scroll_message(msg_scrolling);
        lvgl_port_unlock();

        esp_wifi_set_mode(WIFI_MODE_STA);
        esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
        esp_wifi_start();
        esp_wifi_connect();

        for (int i = 0; (i < WIFI_TIMEOUT_SEC) && !wifi_connected; i++)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        if (wifi_connected)
        {
            ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Connected (stored creds)");
            return true;
        }
    }

    // If stored credentials failed or don't exist, try ArtLogicIoT (manufacturer network)
    // ARTLOGIC_SSID and ARTLOGIC_PASSWORD are set from .env via CMake
    if (strlen(ARTLOGIC_SSID) > 0 && strlen(ARTLOGIC_PASSWORD) > 0)
    {
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Prepare provisioning portal");
        wifi_config_t artlogic_config = {0};
        strncpy((char *)artlogic_config.sta.ssid, ARTLOGIC_SSID, sizeof(artlogic_config.sta.ssid));
        strncpy((char *)artlogic_config.sta.password, ARTLOGIC_PASSWORD, sizeof(artlogic_config.sta.password));

        snprintf(msg_scrolling, SCROLL_MSG_LENGTH, "Preparing provisioning portal");
        lvgl_port_lock(0);
        set_scroll_message(msg_scrolling);
        lvgl_port_unlock();

        esp_wifi_set_mode(WIFI_MODE_STA);
        esp_wifi_set_config(WIFI_IF_STA, &artlogic_config);
        esp_wifi_start();
        esp_wifi_connect();
        manufacturer_mode = true;

        // Wait for 15 seconds to connect to ArtLogicIoT
        for (int i = 0; i < 15 && !wifi_connected; i++)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        if (wifi_connected)
            return true;
    }

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "All connections failed, start provisioning");
    manufacturer_mode = false;
    return false; // This will trigger the provisioning portal
}

void start_softap_provisioning(void)
{
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Start softAP provisioning");
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = AP_SSID,
            .ssid_len = strlen(AP_SSID),
            .password = AP_PASS,
            .max_connection = 4,
            .authmode = WIFI_AUTH_OPEN},
    };
    // Set WiFi mode and configuration
    esp_wifi_set_mode(WIFI_MODE_APSTA); // Change to APSTA mode to support scanning
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);

    // Set optimal channel and bandwidth BEFORE wifi_start
    esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_BW_HT20);

    // Configure IP info for AP mode
    esp_netif_ip_info_t ip_info;
    IP4_ADDR(&ip_info.ip, portal_address[0], portal_address[1], portal_address[2], portal_address[3]);
    IP4_ADDR(&ip_info.gw, portal_address[0], portal_address[1], portal_address[2], portal_address[3]);
    IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);

    esp_netif_dhcps_stop(ap_netif);
    // Configure DHCP server
    esp_netif_set_ip_info(ap_netif, &ip_info);
    // Set DNS server IP (Option 6)
    esp_netif_dhcps_option(ap_netif, ESP_NETIF_OP_SET, ESP_NETIF_DOMAIN_NAME_SERVER,
                           portal_address, sizeof(portal_address));

    // DHCP Captive Portal (Option 114) code removed

    esp_netif_dhcps_start(ap_netif);

    // Start WiFi after configuration
    esp_wifi_start();

    // Don't force channel setting as it may fail if STA is scanning
    // Just log error instead of crashing
    esp_err_t err = esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "WiFi channel set failed: %s", esp_err_to_name(err));
    }
}

void provision_init(void)
{   
    // wifi parameters read from NVS at startup

    if (!connect_to_wifi())
    {
        ESP_LOGI_STACK(TAG, "Launching portal and softAP");

        show_qr_code();


        // Properly clean up mDNS before switching to AP mode
        extern bool mdns_initialized;
        if (mdns_initialized)
        {
            ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Stop mDNS for AP mode");
            mdns_free();
            mdns_initialized = false;
        }

        // Always stop WiFi before starting softAP provisioning
        esp_wifi_stop();
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Stopped WiFi attempts");

        start_softap_provisioning();

        // Start DNS server for captive portal
        esp_ip4_addr_t gateway = {
            .addr = PP_HTONL(LWIP_MAKEU32(portal_address[0], portal_address[1], portal_address[2], portal_address[3]))};

        dns_server_t *dns_server = dns_server_create();
        if (dns_server != NULL)
        {
            if (dns_server_start(dns_server, gateway))
            {
                ESP_LOG_WEB(ESP_LOG_INFO, TAG, "DNS server started");
            }
            else
            {
                ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "DNS server start failed");
            }
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "DNS server create failed");
        }

        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Start HTTP server");
        init_settings_server();

        // Start provisioning timeout timer
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Provisioning timeout %d min", PROVISIONING_TIMEOUT_MINUTES);
        vTaskDelay(pdMS_TO_TICKS(PROVISIONING_TIMEOUT_MS));
        
        // If we're still in provisioning mode after timeout, restart the device
        if (!wifi_connected) {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Provisioning timeout %d min, restart", PROVISIONING_TIMEOUT_MINUTES);
            esp_restart();
        }

        ESP_LOGI_STACK(TAG, "After HTTP server");
    }
    else
    {
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "WiFi connected, settings updated");
        init_settings_server();
        vTaskDelay(pdMS_TO_TICKS(3000));
        settings_updated = true; // trigger a screen update
    }
}