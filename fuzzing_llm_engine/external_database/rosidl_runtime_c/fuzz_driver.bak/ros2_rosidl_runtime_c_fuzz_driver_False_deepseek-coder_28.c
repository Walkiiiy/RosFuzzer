#include "rosidl_runtime_c/message_type_support.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"
#include "rosidl_runtime_c/sequence_bound.h"
#include "rosidl_runtime_c/service_type_support.h"
#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_runtime_c/u16string_functions.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

// LLVMFuzzerTestOneInput function signature
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early return if no data or insufficient size for meaningful operations
    if (!data || size == 0) {
        return 0;
    }

    // Initialize a sequence of UTF-16 strings
    rosidl_runtime_c__U16String__Sequence sequence;
    
    // Determine sequence size from fuzz input (limit to reasonable size)
    // Use first byte modulo 8 to get a small sequence size (0-7)
    size_t seq_size = data[0] % 8;
    
    // Initialize the sequence
    if (!rosidl_runtime_c__U16String__Sequence__init(&sequence, seq_size)) {
        // Initialization failed, nothing to clean up
        return 0;
    }

    // Process each string in the sequence if we have data
    for (size_t i = 0; i < sequence.size && i < size; i++) {
        // Calculate how many bytes to use for this string
        // Ensure we don't exceed available data
        size_t bytes_available = size - i;
        
        // Use a portion of the data for this string
        // Limit to reasonable size and ensure even number of bytes
        size_t bytes_to_use = bytes_available;
        if (bytes_to_use > 256) {
            bytes_to_use = 256;  // Reasonable limit
        }
        
        // Ensure even number of bytes for UTF-16
        if (bytes_to_use % 2 != 0) {
            bytes_to_use--;
        }
        
        // Skip if no bytes available after adjustment
        if (bytes_to_use == 0) {
            continue;
        }

        // Assign data to the UTF-16 string
        // Note: We're casting const uint8_t* to const char* which is safe
        // as we're treating it as a byte array
        if (!rosidl_runtime_c__U16String__assignn_from_char(
                &sequence.data[i], 
                (const char*)(data + i), 
                bytes_to_use)) {
            // Assignment failed, but we continue with other strings
            // The string remains in its initialized state
        }
    }

    // Clean up the sequence (this will also clean up all individual strings)
    rosidl_runtime_c__U16String__Sequence__fini(&sequence);

    return 0;
}

// Main function for standalone testing (not used during fuzzing)
#ifdef STANDALONE_TEST
#include <stdio.h>

int main(int argc, char **argv) {
    // Test with sample data
    uint8_t test_data[] = {0x48, 0x00, 0x65, 0x00, 0x6C, 0x00, 0x6C, 0x00, 0x6F, 0x00};  // "Hello" in UTF-16LE
    size_t test_size = sizeof(test_data);
    
    printf("Running standalone test...\n");
    int result = LLVMFuzzerTestOneInput(test_data, test_size);
    printf("Test completed with result: %d\n", result);
    
    // Test with empty input
    printf("\nTesting with empty input...\n");
    result = LLVMFuzzerTestOneInput(NULL, 0);
    printf("Test completed with result: %d\n", result);
    
    // Test with small input
    printf("\nTesting with small input...\n");
    uint8_t small_data[] = {0x01};
    result = LLVMFuzzerTestOneInput(small_data, 1);
    printf("Test completed with result: %d\n", result);
    
    return 0;
}
#endif
