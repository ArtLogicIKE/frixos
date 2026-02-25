# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

# Generate config.h from .env before components are processed
set(WEATHER_API_KEY "")
set(ARTLOGIC_SSID "")
set(ARTLOGIC_PASSWORD "")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.env")
    file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/.env" ENV_LINES)
    foreach(LINE ${ENV_LINES})
        if(LINE MATCHES "^[ \t]*WEATHER_API_KEY[ \t]*=[ \t]*(.*)$")
            string(STRIP "${CMAKE_MATCH_1}" WEATHER_API_KEY)
            string(REGEX REPLACE "^[\"']|[\"']$" "" WEATHER_API_KEY "${WEATHER_API_KEY}")
        elseif(LINE MATCHES "^[ \t]*ARTLOGIC_SSID[ \t]*=[ \t]*(.*)$")
            string(STRIP "${CMAKE_MATCH_1}" ARTLOGIC_SSID)
            string(REGEX REPLACE "^[\"']|[\"']$" "" ARTLOGIC_SSID "${ARTLOGIC_SSID}")
        elseif(LINE MATCHES "^[ \t]*ARTLOGIC_PASSWORD[ \t]*=[ \t]*(.*)$")
            string(STRIP "${CMAKE_MATCH_1}" ARTLOGIC_PASSWORD)
            string(REGEX REPLACE "^[\"']|[\"']$" "" ARTLOGIC_PASSWORD "${ARTLOGIC_PASSWORD}")
        endif()
    endforeach()
endif()
if(NOT WEATHER_API_KEY)
    message(STATUS "WEATHER_API_KEY not set. Copy env.example to .env and add your OpenWeatherMap API key.")
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/main/config.h.in
               ${CMAKE_CURRENT_SOURCE_DIR}/main/include/config.h @ONLY)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(frixos)
spiffs_create_partition_image(spiffs ./spiffs FLASH_IN_PROJECT)

# Remove the idf_component_register call from here - it belongs only in component directories