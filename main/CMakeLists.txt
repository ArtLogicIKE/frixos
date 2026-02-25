# config.h is generated from .env by top-level CMakeLists.txt before components load

idf_component_register(SRCS 
    "main.c"
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
    REQUIRES esp_http_client
    REQUIRES wifi_provisioning
    REQUIRES json
    REQUIRES app_update
    REQUIRES spi_flash)

    target_compile_options(${COMPONENT_LIB} PRIVATE "-fno-jump-tables" "-fno-tree-switch-conversion")

