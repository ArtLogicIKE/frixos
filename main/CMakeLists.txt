# config.h is generated from .env by top-level CMakeLists.txt before components load

idf_component_register(SRCS 
    "main.c"
    "f-screen-layout.c"
    "f-screen-layout-bin.c"
    "f-wifi.c"
    "f-display.c"
    "f-time.c"
    "f-pwm.c"
    "ltr303.c"  
    "moon.c"
    "f-integrations.c"
    "f-settings.c"
    "f-provisioning.c"
    "f-ota.c"     
    "f-dns.c"    
    "frixos_8.c"
    "frixos_9.c"
    "frixos_10.c"
    "frixos_11.c"
    "frixos_12.c"
    "frixos_14.c"
    "frixos_16.c"
    "font-pixelate_10.c"
    "f-membuffer.c"
    "f-json.c"
    "f-dexcom.c"
    "f-freestyle.c"
    "f-nightscout.c"
    "f-stocks.c"
    # "../managed_components/lvgl__lvgl/src/font/lv_font_montserrat_8.c"
    INCLUDE_DIRS "include"
    INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include"
    REQUIRES spiffs
    REQUIRES esp_wifi
    REQUIRES nvs_flash
    REQUIRES esp_https_server
    REQUIRES esp_http_server
    REQUIRES esp_http_client
    REQUIRES app_update
    REQUIRES spi_flash
    REQUIRES esp_driver_gpio
    REQUIRES esp_driver_i2c
    REQUIRES esp_driver_spi
    REQUIRES esp_driver_ledc
    REQUIRES esp_timer
    REQUIRES esp_lcd
    REQUIRES esp_netif
    REQUIRES esp_event
    REQUIRES lwip
    REQUIRES mbedtls
    REQUIRES esp-tls
    REQUIRES vfs
    REQUIRES teriyakigod__esp_lcd_st7735)

    target_compile_options(${COMPONENT_LIB} PRIVATE "-fno-jump-tables" "-fno-tree-switch-conversion")

