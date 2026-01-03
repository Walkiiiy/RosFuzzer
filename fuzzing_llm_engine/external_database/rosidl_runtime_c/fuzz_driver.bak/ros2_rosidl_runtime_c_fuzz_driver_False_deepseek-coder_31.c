#include "rosidl_runtime_c/message_type_support.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"
#include "rosidl_runtime_c/sequence_bound.h"
#include "rosidl_runtime_c/service_type_support.h"
#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_runtime_c/u16string_functions.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sstream>

// LLVMFuzzerTestOneInput function signature
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early return if input is too small or null
    if (data == NULL || size < sizeof(uint16_t)) {
        return 0;
    }

    // Initialize two U16String structures
    rosidl_runtime_c__U16String str1;
    rosidl_runtime_c__U16String str2;
    rosidl_runtime_c__U16String str3;

    // Initialize str1
    if (!rosidl_runtime_c__U16String__init(&str1)) {
        return 0; // Initialization failed
    }

    // Initialize str2
    if (!rosidl_runtime_c__U16String__init(&str2)) {
        rosidl_runtime_c__U16String__fini(&str1);
        return 0; // Initialization failed
    }

    // Initialize str3
    if (!rosidl_runtime_c__U16String__init(&str3)) {
        rosidl_runtime_c__U16String__fini(&str1);
        rosidl_runtime_c__U16String__fini(&str2);
        return 0; // Initialization failed
    }

    // Calculate safe size for uint16_t array (prevent overflow)
    size_t max_u16_elements = size / sizeof(uint16_t);
    if (max_u16_elements == 0) {
        max_u16_elements = 1; // Ensure at least one element
    }

    // Create a uint16_t array from fuzz input
    // We'll use the first portion of data as UTF-16 content
    uint16_t *u16_data = (uint16_t *)malloc(max_u16_elements * sizeof(uint16_t));
    if (!u16_data) {
        rosidl_runtime_c__U16String__fini(&str1);
        rosidl_runtime_c__U16String__fini(&str2);
        rosidl_runtime_c__U16String__fini(&str3);
        return 0; // Memory allocation failed
    }

    // Copy data from input, ensuring we don't read beyond bounds
    size_t copy_size = (size < max_u16_elements * sizeof(uint16_t)) ? 
                       size : max_u16_elements * sizeof(uint16_t);
    memcpy(u16_data, data, copy_size);

    // Ensure null termination for safety (if we have space)
    if (max_u16_elements > 0) {
        // Find the last element and ensure it's null-terminated if possible
        size_t last_idx = (copy_size / sizeof(uint16_t)) - 1;
        if (last_idx < max_u16_elements) {
            u16_data[last_idx] = 0;
        }
    }

    // Test rosidl_runtime_c__U16String__assign with str1
    if (!rosidl_runtime_c__U16String__assign(&str1, u16_data)) {
        // Assignment failed, clean up and continue
        free(u16_data);
        rosidl_runtime_c__U16String__fini(&str1);
        rosidl_runtime_c__U16String__fini(&str2);
        rosidl_runtime_c__U16String__fini(&str3);
        return 0;
    }

    // Test rosidl_runtime_c__U16String__copy from str1 to str2
    if (!rosidl_runtime_c__U16String__copy(&str1, &str2)) {
        // Copy failed, clean up and continue
        free(u16_data);
        rosidl_runtime_c__U16String__fini(&str1);
        rosidl_runtime_c__U16String__fini(&str2);
        rosidl_runtime_c__U16String__fini(&str3);
        return 0;
    }

    // Test rosidl_runtime_c__U16String__are_equal on str1 and str2
    // They should be equal after copy
    bool are_equal = rosidl_runtime_c__U16String__are_equal(&str1, &str2);
    (void)are_equal; // Use result to avoid unused variable warning

    // Test with str3 (empty string) - should not be equal
    bool not_equal = rosidl_runtime_c__U16String__are_equal(&str1, &str3);
    (void)not_equal; // Use result to avoid unused variable warning

    // Test with NULL pointers for safety
    bool null_check1 = rosidl_runtime_c__U16String__are_equal(NULL, &str1);
    bool null_check2 = rosidl_runtime_c__U16String__are_equal(&str1, NULL);
    bool null_check3 = rosidl_runtime_c__U16String__are_equal(NULL, NULL);
    (void)null_check1; (void)null_check2; (void)null_check3;

    // Test copy with NULL input
    bool copy_null = rosidl_runtime_c__U16String__copy(NULL, &str3);
    (void)copy_null;

    // Clean up allocated memory
    free(u16_data);

    // Finalize all strings
    rosidl_runtime_c__U16String__fini(&str1);
    rosidl_runtime_c__U16String__fini(&str2);
    rosidl_runtime_c__U16String__fini(&str3);

    return 0; // Always return 0
}
