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
    if (!data || size == 0) {
        return 0;
    }

    // Initialize two U16String structures for testing
    rosidl_runtime_c__U16String str1;
    rosidl_runtime_c__U16String str2;
    
    // Initialize str1
    if (!rosidl_runtime_c__U16String__init(&str1)) {
        // Initialization failed, nothing to free
        return 0;
    }
    
    // Initialize str2
    if (!rosidl_runtime_c__U16String__init(&str2)) {
        // Clean up str1 before returning
        rosidl_runtime_c__U16String__fini(&str1);
        return 0;
    }

    // Test rosidl_runtime_c__U16String__assignn_from_char
    // Ensure we have at least 2 bytes and make n even
    size_t assign_size = size;
    if (assign_size % 2 != 0) {
        assign_size--;  // Make it even
    }
    
    // Ensure we have at least 2 bytes after adjustment
    if (assign_size >= 2) {
        // Cast data to char* for assignn_from_char
        const char* char_data = (const char*)data;
        
        // Test assignment to str1
        if (!rosidl_runtime_c__U16String__assignn_from_char(&str1, char_data, assign_size)) {
            // Assignment failed, but we continue with other tests
            // Reset str1 to empty state
            rosidl_runtime_c__U16String__fini(&str1);
            if (!rosidl_runtime_c__U16String__init(&str1)) {
                rosidl_runtime_c__U16String__fini(&str2);
                return 0;
            }
        }
        
        // Test assignment to str2 with potentially different size
        // Use at most half of assign_size to test different lengths
        size_t assign_size2 = assign_size / 2;
        if (assign_size2 % 2 != 0) {
            assign_size2--;  // Make it even
        }
        
        if (assign_size2 >= 2) {
            if (!rosidl_runtime_c__U16String__assignn_from_char(&str2, char_data, assign_size2)) {
                // Assignment failed, reset str2
                rosidl_runtime_c__U16String__fini(&str2);
                if (!rosidl_runtime_c__U16String__init(&str2)) {
                    rosidl_runtime_c__U16String__fini(&str1);
                    return 0;
                }
            }
        }
    }

    // Test rosidl_runtime_c__U16String__len
    // This function expects a null-terminated uint16_t array
    // We'll use the data from str1 if it has been assigned
    size_t len1 = 0;
    if (str1.data) {
        len1 = rosidl_runtime_c__U16String__len(str1.data);
    }
    
    size_t len2 = 0;
    if (str2.data) {
        len2 = rosidl_runtime_c__U16String__len(str2.data);
    }

    // Test rosidl_runtime_c__U16String__are_equal
    bool are_equal = rosidl_runtime_c__U16String__are_equal(&str1, &str2);
    
    // Test rosidl_runtime_c__U16String__resize
    // Use fuzz input to determine new size, but bound it reasonably
    size_t new_size = size % 256;  // Limit to reasonable size for fuzzing
    
    // Try to resize str1
    bool resize_success = rosidl_runtime_c__U16String__resize(&str1, new_size);
    
    // If resize succeeded, we can test len again
    if (resize_success && str1.data) {
        size_t new_len = rosidl_runtime_c__U16String__len(str1.data);
        // new_len should be <= new_size (could be less if there are null characters)
        (void)new_len;  // Mark as used to avoid compiler warning
    }
    
    // Also test resizing str2 to a different size
    size_t new_size2 = (size / 2) % 128;  // Different size for str2
    bool resize_success2 = rosidl_runtime_c__U16String__resize(&str2, new_size2);
    
    // Test equality again after resizing
    bool are_equal_after_resize = rosidl_runtime_c__U16String__are_equal(&str1, &str2);
    
    // Mark variables as used to avoid compiler warnings
    (void)len1;
    (void)len2;
    (void)are_equal;
    (void)resize_success;
    (void)resize_success2;
    (void)are_equal_after_resize;

    // Clean up allocated resources
    rosidl_runtime_c__U16String__fini(&str1);
    rosidl_runtime_c__U16String__fini(&str2);

    return 0;
}

// Note: The rosidl_runtime_c__U16String__fini function is assumed to exist
// based on the pattern in ROS 2 runtime C utilities. If it doesn't exist,
// we would need to implement proper cleanup manually.
// For this fuzzer, we assume it exists and is declared in u16string_functions.h

// If rosidl_runtime_c__U16String__fini is not available, we would need:
/*
static void rosidl_runtime_c__U16String__fini(rosidl_runtime_c__U16String * str) {
    if (str && str->data) {
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        allocator.deallocate(str->data, allocator.state);
        str->data = NULL;
        str->size = 0;
        str->capacity = 0;
    }
}
*/
