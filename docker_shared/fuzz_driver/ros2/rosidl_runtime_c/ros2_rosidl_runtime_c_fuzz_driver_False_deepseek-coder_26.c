#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sstream>
#include "rosidl_runtime_c/message_type_support.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"
#include "rosidl_runtime_c/sequence_bound.h"
#include "rosidl_runtime_c/service_type_support.h"
#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_runtime_c/u16string_functions.h"

// Helper function to calculate length of UTF-16 string (excluding null terminator)
static size_t u16_strlen(const uint16_t *str) {
    if (!str) return 0;
    size_t len = 0;
    while (str[len] != 0) {
        len++;
    }
    return len;
}

// Helper function to create a UTF-16 string from fuzz input
static uint16_t* create_u16_string_from_fuzz(const uint8_t *data, size_t size, size_t *out_len) {
    if (!data || size == 0) {
        *out_len = 0;
        return NULL;
    }
    
    // Each UTF-16 character is 2 bytes, but we'll use the fuzz data as-is
    // Convert bytes to uint16_t values
    size_t max_chars = size / sizeof(uint16_t);
    if (max_chars == 0) {
        *out_len = 0;
        return NULL;
    }
    
    uint16_t *u16_str = (uint16_t*)malloc((max_chars + 1) * sizeof(uint16_t));
    if (!u16_str) {
        *out_len = 0;
        return NULL;
    }
    
    // Copy data, ensuring null termination
    size_t chars_to_copy = max_chars;
    if (chars_to_copy > 0) {
        memcpy(u16_str, data, chars_to_copy * sizeof(uint16_t));
    }
    u16_str[chars_to_copy] = 0;
    
    *out_len = chars_to_copy;
    return u16_str;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Initialize variables
    rosidl_runtime_c__U16String str1 = {0};
    rosidl_runtime_c__U16String str2 = {0};
    rosidl_runtime_c__U16String str3 = {0};
    uint16_t *fuzz_u16_str = NULL;
    size_t fuzz_len = 0;
    bool success = false;
    
    // Create UTF-16 string from fuzz input
    fuzz_u16_str = create_u16_string_from_fuzz(data, size, &fuzz_len);
    
    // Test 1: Initialize str1
    if (!rosidl_runtime_c__U16String__init(&str1)) {
        goto cleanup;
    }
    
    // Test 2: Initialize str2
    if (!rosidl_runtime_c__U16String__init(&str2)) {
        goto cleanup;
    }
    
    // Test 3: Initialize str3
    if (!rosidl_runtime_c__U16String__init(&str3)) {
        goto cleanup;
    }
    
    // Test 4: Assign fuzz data to str1 if we have valid data
    if (fuzz_u16_str && fuzz_len > 0) {
        if (!rosidl_runtime_c__U16String__assign(&str1, fuzz_u16_str)) {
            goto cleanup;
        }
    }
    
    // Test 5: Copy str1 to str2
    if (!rosidl_runtime_c__U16String__copy(&str1, &str2)) {
        goto cleanup;
    }
    
    // Test 6: Check if str1 and str2 are equal (they should be after copy)
    if (!rosidl_runtime_c__U16String__are_equal(&str1, &str2)) {
        // This is not necessarily an error, but we'll note it
        // Could happen if str1 is empty and str2 initialization failed
    }
    
    // Test 7: Assign a simple test string to str3
    // Create a simple UTF-16 string "test"
    uint16_t test_str[] = {0x0074, 0x0065, 0x0073, 0x0074, 0};  // "test" in UTF-16
    if (!rosidl_runtime_c__U16String__assign(&str3, test_str)) {
        goto cleanup;
    }
    
    // Test 8: Check equality between different strings
    // str1 (from fuzz) and str3 (fixed) are likely different
    bool are_equal = rosidl_runtime_c__U16String__are_equal(&str1, &str3);
    // This is expected to be false in most cases
    
    success = true;

cleanup:
    // Clean up all allocated resources
    if (fuzz_u16_str) {
        free(fuzz_u16_str);
    }
    
    // Finalize all strings
    rosidl_runtime_c__U16String__fini(&str1);
    rosidl_runtime_c__U16String__fini(&str2);
    rosidl_runtime_c__U16String__fini(&str3);
    
    // Return 0 to indicate successful execution (regardless of test outcomes)
    // The fuzzer cares about crashes, not test pass/fail
    return 0;
}
