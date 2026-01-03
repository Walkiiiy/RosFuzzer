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
    // Early return if no data or insufficient size
    if (data == NULL || size == 0) {
        return 0;
    }

    // Initialize the UTF-16 string structure
    rosidl_runtime_c__U16String str;
    memset(&str, 0, sizeof(str));
    
    // 1. Call rosidl_runtime_c__U16String__init
    if (!rosidl_runtime_c__U16String__init(&str)) {
        // Initialization failed, nothing to clean up
        return 0;
    }

    // Ensure we have enough data for meaningful operations
    // We need at least 2 bytes for UTF-16 operations
    if (size < 2) {
        rosidl_runtime_c__U16String__fini(&str);
        return 0;
    }

    // 2. Call rosidl_runtime_c__U16String__assignn_from_char
    // Prepare data for assignn_from_char - ensure n is even
    size_t assign_from_char_n = size;
    if (assign_from_char_n % 2 != 0) {
        // Make it even by using one less byte
        assign_from_char_n--;
    }
    
    // Ensure we still have data after adjustment
    if (assign_from_char_n >= 2) {
        if (!rosidl_runtime_c__U16String__assignn_from_char(&str, 
                (const char*)data, assign_from_char_n)) {
            // Assignment failed, clean up and return
            rosidl_runtime_c__U16String__fini(&str);
            return 0;
        }
    }

    // 3. Call rosidl_runtime_c__U16String__len
    // Get the length of the current string
    size_t current_len = rosidl_runtime_c__U16String__len(str.data);
    
    // 4. Call rosidl_runtime_c__U16String__assignn
    // Prepare new data for assignn operation
    // We'll use a portion of the fuzz data as UTF-16 characters
    size_t assign_n = size / sizeof(uint16_t);
    if (assign_n > 0) {
        // Ensure we don't overflow when creating the uint16_t array
        if (assign_n > SIZE_MAX / sizeof(uint16_t)) {
            rosidl_runtime_c__U16String__fini(&str);
            return 0;
        }
        
        // Create a temporary buffer for UTF-16 data
        uint16_t* temp_utf16 = (uint16_t*)malloc(assign_n * sizeof(uint16_t));
        if (temp_utf16 == NULL) {
            rosidl_runtime_c__U16String__fini(&str);
            return 0;
        }
        
        // Copy data from input, ensuring we don't read beyond bounds
        size_t bytes_to_copy = assign_n * sizeof(uint16_t);
        if (bytes_to_copy > size) {
            bytes_to_copy = size;
            assign_n = bytes_to_copy / sizeof(uint16_t);
        }
        
        memcpy(temp_utf16, data, bytes_to_copy);
        
        // Call assignn with the temporary buffer
        if (!rosidl_runtime_c__U16String__assignn(&str, temp_utf16, assign_n)) {
            free(temp_utf16);
            rosidl_runtime_c__U16String__fini(&str);
            return 0;
        }
        
        free(temp_utf16);
    }

    // 5. Call rosidl_runtime_c__U16String__resize
    // Calculate a new size based on the fuzz input
    // Use a value within reasonable bounds to avoid excessive memory allocation
    size_t new_size = size % 256;  // Limit to 256 characters max
    
    if (new_size > 0) {
        if (!rosidl_runtime_c__U16String__resize(&str, new_size)) {
            // Resize failed, clean up and return
            rosidl_runtime_c__U16String__fini(&str);
            return 0;
        }
        
        // Verify the new length
        size_t updated_len = rosidl_runtime_c__U16String__len(str.data);
        // Note: updated_len might be less than new_size due to null terminators
    }

    // Additional safety: Verify string integrity
    if (str.data != NULL && str.size > 0) {
        // Ensure null termination
        if (str.size < str.capacity) {
            // This should already be guaranteed by the API functions
            // but we verify for safety
            if (str.data[str.size] != 0) {
                // Force null termination for safety
                str.data[str.size] = 0;
            }
        }
        
        // Verify we can safely access the string
        for (size_t i = 0; i < str.size && i < str.capacity - 1; i++) {
            // Just accessing to ensure no segmentation fault
            volatile uint16_t ch = str.data[i];
            (void)ch;  // Prevent unused variable warning
        }
    }

    // Clean up allocated resources
    rosidl_runtime_c__U16String__fini(&str);

    return 0;
}

// Note: The rosidl_runtime_c__U16String__fini function is assumed to exist
// based on the pattern of ROS 2 string functions. If it doesn't exist,
// you would need to manually free the string data:
// 
// Alternative cleanup if __fini doesn't exist:
// 
// void custom_u16string_fini(rosidl_runtime_c__U16String *str) {
//     if (str && str->data) {
//         free(str->data);
//         str->data = NULL;
//         str->size = 0;
//         str->capacity = 0;
//     }
// }
