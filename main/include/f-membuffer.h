#ifndef F_MEMBUFFER_H
#define F_MEMBUFFER_H

#include "frixos.h"
#include <stddef.h>
#include <stdbool.h>

// Buffer pool configuration
#define MAX_BUFFER_POOL_SIZE 4  // Max buffers to allocate - should be enough for all integrations
#define HTTP_BUFFER_SIZE 4096
#define BUFFER_POOL_TIMEOUT_MS 1000

// Buffer pool structure
typedef struct {
    char* buffer;
    size_t size;
    bool in_use;
    int64_t last_used;
    const char* owner;
} buffer_pool_t;

// Buffer management functions
bool init_buffer_management(void);
void cleanup_buffer_management(void);
char* get_shared_buffer(size_t required_size, const char* owner);
void release_shared_buffer(char* buffer);
void get_buffer_pool_stats(size_t* total_buffers, size_t* used_buffers, size_t* total_memory);

#endif // F_MEMBUFFER_H 