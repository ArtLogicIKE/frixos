#ifndef F_JSON_H
#define F_JSON_H

#include <stddef.h>

/**
 * @brief Extract a value from a JSON string by key
 * 
 * @param json_str The JSON string to search in
 * @param key The key to look for
 * @param output Buffer to store the result
 * @param output_size Size of the output buffer
 * @param remaining_str Pointer to store the position after the found key (can be NULL)
 * @return char* Pointer to the output buffer containing the value, or "-" if not found
 */
char *get_value_from_JSON_string(const char *json_str, const char *key, char *output, size_t output_size, char **remaining_str);

#endif // F_JSON_H 