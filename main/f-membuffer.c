#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "f-membuffer.h"

static const char *TAG = "f-membuffer";

// Statically allocate the buffer pool
static char static_buffers[MAX_BUFFER_POOL_SIZE][HTTP_BUFFER_SIZE] __attribute__((aligned(4)));
static buffer_pool_t global_buffer_pool[MAX_BUFFER_POOL_SIZE] = {0};
static SemaphoreHandle_t buffer_mutex = NULL;

// Initialize buffer pool
static bool init_buffer_pool(void) {
    for (int i = 0; i < MAX_BUFFER_POOL_SIZE; i++) {
        global_buffer_pool[i].buffer = static_buffers[i];
        global_buffer_pool[i].size = HTTP_BUFFER_SIZE;
        global_buffer_pool[i].in_use = false;
        global_buffer_pool[i].last_used = 0;
        global_buffer_pool[i].owner = NULL;
    }
    return true;
}

// Cleanup buffer pool
static void cleanup_buffer_pool(void) {
    for (int i = 0; i < MAX_BUFFER_POOL_SIZE; i++) {
        global_buffer_pool[i].buffer = NULL;
        global_buffer_pool[i].size = 0;
        global_buffer_pool[i].in_use = false;
        global_buffer_pool[i].last_used = 0;
        global_buffer_pool[i].owner = NULL;
    }
}

// Initialize buffer management system
bool init_buffer_management(void) {
    if (!init_buffer_pool()) {
        return false;
    }
    buffer_mutex = xSemaphoreCreateMutex();
    if (buffer_mutex == NULL) {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Buffer mutex create failed during init");
        return false;
    }
    return true;
}

// Cleanup buffer management system
void cleanup_buffer_management(void) {
    cleanup_buffer_pool();
    if (buffer_mutex != NULL) {
        vSemaphoreDelete(buffer_mutex);
        buffer_mutex = NULL;
    }
}

// Get a buffer from the pool
char* get_shared_buffer(size_t required_size, const char* owner) {
    if (buffer_mutex == NULL) {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Buffer mutex not initialized - call init_buffer_management() first");
        return NULL;
    }

    if (xSemaphoreTake(buffer_mutex, pdMS_TO_TICKS(BUFFER_POOL_TIMEOUT_MS)) != pdTRUE) {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to take buffer mutex");
        return NULL;
    }

    // First try to find an unused buffer
    for (int i = 0; i < MAX_BUFFER_POOL_SIZE; i++) {
        if (!global_buffer_pool[i].in_use && global_buffer_pool[i].buffer != NULL && 
            global_buffer_pool[i].size >= required_size) {
            global_buffer_pool[i].in_use = true;
            global_buffer_pool[i].last_used = esp_timer_get_time();
            global_buffer_pool[i].owner = owner;
            xSemaphoreGive(buffer_mutex);
            return global_buffer_pool[i].buffer;
        }
    }

    xSemaphoreGive(buffer_mutex);
    if (required_size > HTTP_BUFFER_SIZE) {
        // Pool buffers are static: they can never be freed or grown.
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Buffer request %u > pool size %u (%s)",
                    (unsigned)required_size, (unsigned)HTTP_BUFFER_SIZE, owner ? owner : "?");
        return NULL;
    }
    ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "**** No buffer found in pool - buffers exhausted ****");
    return NULL;
}

// Release a buffer back to the pool
void release_shared_buffer(char* buffer) {
    if (buffer_mutex == NULL || buffer == NULL) return;

    if (xSemaphoreTake(buffer_mutex, pdMS_TO_TICKS(BUFFER_POOL_TIMEOUT_MS)) != pdTRUE) {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Buffer mutex release failed");
        return;
    }

    for (int i = 0; i < MAX_BUFFER_POOL_SIZE; i++) {
        if (global_buffer_pool[i].buffer == buffer) {
            global_buffer_pool[i].in_use = false;
            global_buffer_pool[i].owner = NULL;
            break;
        }
    }

    xSemaphoreGive(buffer_mutex);
}

// Get buffer pool statistics
void get_buffer_pool_stats(size_t* total_buffers, size_t* used_buffers, size_t* total_memory) {
    if (buffer_mutex == NULL) {
        *total_buffers = 0;
        *used_buffers = 0;
        *total_memory = 0;
        return;
    }

    if (xSemaphoreTake(buffer_mutex, pdMS_TO_TICKS(BUFFER_POOL_TIMEOUT_MS)) != pdTRUE) {
        *total_buffers = 0;
        *used_buffers = 0;
        *total_memory = 0;
        return;
    }

    *total_buffers = MAX_BUFFER_POOL_SIZE;
    *used_buffers = 0;
    *total_memory = 0;

    for (int i = 0; i < MAX_BUFFER_POOL_SIZE; i++) {
        if (global_buffer_pool[i].buffer != NULL) {
            *total_memory += global_buffer_pool[i].size;
            if (global_buffer_pool[i].in_use) {
                (*used_buffers)++;
            }
        }
    }

    xSemaphoreGive(buffer_mutex);
} 