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
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early return if no data or insufficient size for meaningful operations
    if (!data || size == 0) {
        return 0;
    }

    // Initialize two U16String structures for testing
    rosidl_runtime_c__U16String str1;
    rosidl_runtime_c__U16String str2;
    
    // Initialize str1
    if (!rosidl_runtime_c__U16String__init(&str1)) {
        // Initialization failed, nothing to clean up
        return 0;
    }
    
    // Initialize str2
    if (!rosidl_runtime_c__U16String__init(&str2)) {
        // Clean up str1 before returning
        rosidl_runtime_c__U16String__fini(&str1);
        return 0;
    }
    
    // Use fuzz data for assignn_from_char operations
    // Ensure we have at least 2 bytes for UTF-16 (n must be even)
    size_t assign_size = size;
    if (assign_size % 2 != 0) {
        // Make size even by using one less byte
        assign_size = size - 1;
    }
    
    // Ensure we have at least 2 bytes for assignment
    if (assign_size >= 2) {
        // Assign to str1 using the fuzz data
        if (!rosidl_runtime_c__U16String__assignn_from_char(&str1, (const char*)data, assign_size)) {
            // Assignment failed, clean up and return
            rosidl_runtime_c__U16String__fini(&str1);
            rosidl_runtime_c__U16String__fini(&str2);
            return 0;
        }
        
        // Assign the same data to str2 to test equality
        if (!rosidl_runtime_c__U16String__assignn_from_char(&str2, (const char*)data, assign_size)) {
            // Assignment failed, clean up and return
            rosidl_runtime_c__U16String__fini(&str1);
            rosidl_runtime_c__U16String__fini(&str2);
            return 0;
        }
        
        // Test equality - should return true since both strings have same content
        bool are_equal = rosidl_runtime_c__U16String__are_equal(&str1, &str2);
        (void)are_equal; // Prevent unused variable warning
        
        // Test inequality by modifying str2 if possible
        if (str2.size > 0) {
            // Create a modified version by changing the first character
            uint16_t original_first = str2.data[0];
            str2.data[0] = original_first + 1; // Simple modification
            
            // Test equality again - should return false
            bool are_not_equal = rosidl_runtime_c__U16String__are_equal(&str1, &str2);
            (void)are_not_equal; // Prevent unused variable warning
            
            // Restore original value for proper cleanup
            str2.data[0] = original_first;
        }
    }
    
    // Test sequence creation with size derived from fuzz input
    // Use a bounded size to prevent excessive memory allocation
    size_t sequence_size = size % 10; // Limit to 0-9 elements for safety
    
    // Create a sequence of U16Strings
    rosidl_runtime_c__U16String__Sequence *sequence = 
        rosidl_runtime_c__U16String__Sequence__create(sequence_size);
    
    if (sequence) {
        // Initialize each string in the sequence
        for (size_t i = 0; i < sequence_size; i++) {
            if (!rosidl_runtime_c__U16String__init(&sequence->data[i])) {
                // If initialization fails, clean up already initialized strings
                for (size_t j = 0; j < i; j++) {
                    rosidl_runtime_c__U16String__fini(&sequence->data[j]);
                }
                // Free the sequence structure itself
                // Note: We need to use the default allocator's deallocate function
                // Since we don't have direct access to it, we'll rely on the library's
                // cleanup function if available, or use free() as fallback
                // In practice, rosidl_runtime_c__U16String__Sequence__destroy would be used
                // but we don't have it in the provided API. We'll use free() as a fallback.
                free(sequence);
                sequence = NULL;
                break;
            }
            
            // Assign some data to each string if we have enough fuzz data
            if (size >= 2) {
                // Use a small portion of fuzz data for each string
                size_t str_data_size = 2 * ((i + 1) % 4); // 0, 2, 4, or 6 bytes
                if (str_data_size > 0 && size >= str_data_size) {
                    rosidl_runtime_c__U16String__assignn_from_char(
                        &sequence->data[i], 
                        (const char*)data + (i % (size - str_data_size)), 
                        str_data_size);
                }
            }
        }
        
        // Clean up the sequence if it was created
        if (sequence) {
            // First finalize each string in the sequence
            for (size_t i = 0; i < sequence_size; i++) {
                rosidl_runtime_c__U16String__fini(&sequence->data[i]);
            }
            // Free the sequence structure
            // Note: In a real implementation, we would use 
            // rosidl_runtime_c__U16String__Sequence__destroy(sequence)
            // but since it's not provided, we use free() as fallback
            free(sequence);
        }
    }
    
    // Test equality with NULL pointers (error handling)
    bool null_test1 = rosidl_runtime_c__U16String__are_equal(NULL, &str1);
    bool null_test2 = rosidl_runtime_c__U16String__are_equal(&str1, NULL);
    bool null_test3 = rosidl_runtime_c__U16String__are_equal(NULL, NULL);
    (void)null_test1;
    (void)null_test2;
    (void)null_test3;
    
    // Test assignn_from_char with odd n (should fail)
    if (size >= 3) {
        bool odd_test = rosidl_runtime_c__U16String__assignn_from_char(&str1, (const char*)data, 3);
        (void)odd_test;
    }
    
    // Clean up the individual strings
    rosidl_runtime_c__U16String__fini(&str1);
    rosidl_runtime_c__U16String__fini(&str2);
    
    return 0;
}

// Main function for standalone testing (not used by libFuzzer)
#ifdef STANDALONE_FUZZER
int main(int argc, char **argv) {
    // Simple test with sample data
    const char *test_data = "Hello World!";
    size_t test_size = strlen(test_data);
    
    // Ensure even number of bytes for UTF-16
    if (test_size % 2 != 0) {
        test_size--;
    }
    
    LLVMFuzzerTestOneInput((const uint8_t*)test_data, test_size);
    return 0;
}
#endif
