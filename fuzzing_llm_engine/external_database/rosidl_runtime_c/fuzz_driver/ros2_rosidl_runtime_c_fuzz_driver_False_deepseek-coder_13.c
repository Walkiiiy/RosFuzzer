#include <rosidl_runtime_c/primitives_sequence_functions.h>
#include <rosidl_runtime_c/sequence_bound.h>
#include <rosidl_runtime_c/string_functions.h>
#include <rosidl_runtime_c/u16string_functions.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <rcutils/allocator.h>

// Workaround for missing headers - provide dummy declarations
#ifndef ROSIDL_RUNTIME_C_MESSAGE_TYPE_SUPPORT_H
#define ROSIDL_RUNTIME_C_MESSAGE_TYPE_SUPPORT_H
// Dummy declarations if header is not available
#endif

#ifndef ROSIDL_RUNTIME_C_SERVICE_TYPE_SUPPORT_H  
#define ROSIDL_RUNTIME_C_SERVICE_TYPE_SUPPORT_H
// Dummy declarations if header is not available
#endif

// Function to safely extract a size_t from fuzz data
static size_t extract_size_t(const uint8_t **data, size_t *size) {
    if (*size < sizeof(size_t)) {
        return 0;
    }
    size_t value;
    memcpy(&value, *data, sizeof(size_t));
    *data += sizeof(size_t);
    *size -= sizeof(size_t);
    return value;
}

// Function to safely extract a boolean from fuzz data
static bool extract_bool(const uint8_t **data, size_t *size) {
    if (*size < 1) {
        return false;
    }
    bool value = (*data)[0] & 0x01;
    (*data)++;
    (*size)--;
    return value;
}

// Function to safely extract a string from fuzz data
static char* extract_string(const uint8_t **data, size_t *size, size_t max_len) {
    if (*size == 0) {
        return NULL;
    }
    
    // Determine string length (up to max_len or available data)
    size_t str_len = 0;
    while (str_len < max_len && str_len < *size && (*data)[str_len] != '\0') {
        str_len++;
    }
    
    if (str_len == 0 && *size > 0 && (*data)[0] == '\0') {
        str_len = 1; // Handle empty string case
    }
    
    if (str_len == 0) {
        return NULL;
    }
    
    // Allocate and copy string
    char *str = (char*)malloc(str_len + 1);
    if (!str) {
        return NULL;
    }
    
    size_t copy_len = str_len < *size ? str_len : *size;
    memcpy(str, *data, copy_len);
    str[copy_len] = '\0';
    
    *data += copy_len;
    *size -= copy_len;
    
    return str;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early exit if we don't have enough data for basic operations
    if (size < sizeof(size_t) * 2) {
        return 0;
    }
    
    // Keep original pointer for bounds checking
    const uint8_t *original_data = data;
    size_t original_size = size;
    
    // Extract parameters from fuzz data
    size_t seq_size = extract_size_t(&data, &size);
    if (seq_size > 100) { // Limit sequence size to prevent excessive memory usage
        seq_size = 100;
    }
    
    // Initialize sequences for testing
    rosidl_runtime_c__String__Sequence seq1;
    rosidl_runtime_c__String__Sequence seq2;
    rosidl_runtime_c__boolean__Sequence bool_seq1;
    rosidl_runtime_c__boolean__Sequence bool_seq2;
    
    // Initialize string sequences
    if (!rosidl_runtime_c__String__Sequence__init(&seq1, seq_size)) {
        return 0;
    }
    
    if (!rosidl_runtime_c__String__Sequence__init(&seq2, seq_size)) {
        // Properly clean up seq1 before returning
        rosidl_runtime_c__String__Sequence__fini(&seq1);
        return 0;
    }
    
    // Initialize boolean sequences
    if (!rosidl_runtime_c__boolean__Sequence__init(&bool_seq1, seq_size)) {
        rosidl_runtime_c__String__Sequence__fini(&seq1);
        rosidl_runtime_c__String__Sequence__fini(&seq2);
        return 0;
    }
    
    if (!rosidl_runtime_c__boolean__Sequence__init(&bool_seq2, seq_size)) {
        rosidl_runtime_c__String__Sequence__fini(&seq1);
        rosidl_runtime_c__String__Sequence__fini(&seq2);
        rosidl_runtime_c__boolean__Sequence__fini(&bool_seq1);
        return 0;
    }
    
    // REMOVED: The manual initialization loop since rosidl_runtime_c__String__Sequence__init
    // already initializes all strings in the sequence
    
    // Populate sequences with fuzz data
    for (size_t i = 0; i < seq_size && size > 0; i++) {
        // Extract string data from fuzz input
        char *str_data = extract_string(&data, &size, 100);
        if (str_data) {
            // Resize string to hold the data
            if (!rosidl_runtime_c__String__assignn(&seq1.data[i], str_data, strlen(str_data))) {
                free(str_data);
                goto cleanup;
            }
            
            // For seq2, sometimes use same data, sometimes different
            bool use_same_data = extract_bool(&data, &size);
            if (use_same_data && size > 0) {
                rosidl_runtime_c__String__assign(&seq2.data[i], str_data);
            } else if (size > 0) {
                char *str_data2 = extract_string(&data, &size, 100);
                if (str_data2) {
                    rosidl_runtime_c__String__assign(&seq2.data[i], str_data2);
                    free(str_data2);
                }
            }
            
            free(str_data);
        }
        
        // Populate boolean sequences
        if (size > 0) {
            bool_seq1.data[i] = extract_bool(&data, &size);
            bool_seq2.data[i] = extract_bool(&data, &size);
        }
    }
    
    // Test 1: Compare string sequences
    bool seq_equal = rosidl_runtime_c__String__Sequence__are_equal(&seq1, &seq2);
    (void)seq_equal; // Use result to avoid unused variable warning
    
    // Test 2: Compare individual strings
    if (seq_size > 0) {
        for (size_t i = 0; i < seq_size && i < 5; i++) { // Limit to first 5
            bool str_equal = rosidl_runtime_c__String__are_equal(&seq1.data[i], &seq2.data[i]);
            (void)str_equal; // Use result
        }
    }
    
    // Test 3: Compare boolean sequences
    bool bool_seq_equal = rosidl_runtime_c__bool__Sequence__are_equal(&bool_seq1, &bool_seq2);
    (void)bool_seq_equal; // Use result
    
    // Test 4: Create and compare additional individual strings
    rosidl_runtime_c__String extra_str1, extra_str2;
    if (rosidl_runtime_c__String__init(&extra_str1) && 
        rosidl_runtime_c__String__init(&extra_str2)) {
        
        // Use remaining fuzz data for string content
        if (size > 0) {
            size_t str_len = size > 100 ? 100 : size;
            char *temp_str = (char*)malloc(str_len + 1);
            if (temp_str) {
                memcpy(temp_str, data, str_len);
                temp_str[str_len] = '\0';
                
                rosidl_runtime_c__String__assign(&extra_str1, temp_str);
                
                // Sometimes assign same, sometimes different
                if (str_len > 1 && (data[0] & 0x01)) {
                    rosidl_runtime_c__String__assign(&extra_str2, temp_str);
                } else {
                    // Create slightly different string
                    temp_str[0] = temp_str[0] ^ 0x01; // Flip first bit
                    rosidl_runtime_c__String__assign(&extra_str2, temp_str);
                }
                
                free(temp_str);
            }
        }
        
        // Compare the extra strings
        bool extra_equal = rosidl_runtime_c__String__are_equal(&extra_str1, &extra_str2);
        (void)extra_equal; // Use result
        
        // Cleanup extra strings
        rosidl_runtime_c__String__fini(&extra_str1);
        rosidl_runtime_c__String__fini(&extra_str2);
    }
    
cleanup:
    // Cleanup all allocated resources
    // Note: rosidl_runtime_c__String__Sequence__fini will call fini on all strings in the sequence
    rosidl_runtime_c__String__Sequence__fini(&seq1);
    rosidl_runtime_c__String__Sequence__fini(&seq2);
    rosidl_runtime_c__boolean__Sequence__fini(&bool_seq1);
    rosidl_runtime_c__boolean__Sequence__fini(&bool_seq2);
    
    return 0;
}
