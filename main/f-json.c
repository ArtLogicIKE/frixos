#include "f-json.h"
#include <string.h>
#include <ctype.h>
#include "frixos.h"

char *get_value_from_JSON_string(const char *json_str, const char *key, char *output, size_t output_size, char **remaining_str)
{
    if (!json_str || !key || !output || output_size < 2)
    {
        strcpy(output, "-");
        if (remaining_str) *remaining_str = NULL;
        return output;
    }

    // Construct the search pattern: "key"
    char search_pattern[64];
    snprintf(search_pattern, sizeof(search_pattern), "\"%s\"", key);
    
    // Find the key in the JSON string
    char *key_pos = strstr(json_str, search_pattern);
    if (!key_pos)
    {
        strcpy(output, "-");
        if (remaining_str) *remaining_str = NULL;
        return output;
    }

    // Move past the key
    char *value_start = key_pos + strlen(search_pattern);
    
    // Skip any whitespace and find the colon
    while (*value_start && isspace((unsigned char)*value_start))
    {
        value_start++;
    }

    if (*value_start != ':')
    {
        strcpy(output, "-");
        if (remaining_str) *remaining_str = NULL;
        return output;
    }
    value_start++; // Skip colon

    // Skip any whitespace after colon
    while (*value_start && isspace((unsigned char)*value_start))
    {
        value_start++;
    }

    // Handle different value types
    if (*value_start == '"')
    {
        // String value
        value_start++; // Skip the opening quote
        char *value_end = strchr(value_start, '"');
        if (!value_end)
        {
            strcpy(output, "-");
            if (remaining_str) *remaining_str = NULL;
            return output;
        }
        size_t len = value_end - value_start;
        if (len >= output_size)
        {
            len = output_size - 1;
        }
        strncpy(output, value_start, len);
        output[len] = '\0';
        if (remaining_str) *remaining_str = value_end + 1; // Point after the closing quote
    }
    else if (*value_start == '{' || *value_start == '[')
    {
        // Object or array - return the entire structure
        char *value_end = value_start + 1;
        int brace_count = 1;
        while (*value_end && brace_count > 0)
        {
            if (*value_end == '{' || *value_end == '[')
                brace_count++;
            if (*value_end == '}' || *value_end == ']')
                brace_count--;
            value_end++;
        }
        size_t len = value_end - value_start;
        if (len >= output_size)
        {
            len = output_size - 1;
        }
        strncpy(output, value_start, len);
        output[len] = '\0';
        if (remaining_str) *remaining_str = value_end; // Point after the closing brace/bracket
    }
    else
    {
        // Number or boolean
        char *value_end = value_start;
        while (*value_end && !isspace((unsigned char)*value_end) && *value_end != ',' && *value_end != '}')
        {
            value_end++;
        }
        size_t len = value_end - value_start;
        if (len >= output_size)
        {
            len = output_size - 1;
        }
        strncpy(output, value_start, len);
        output[len] = '\0';
        if (remaining_str) *remaining_str = value_end; // Point after the value
    }

    return output;
} 