#include <rosidl_runtime_c/message_type_support_struct.h>
#include <rosidl_runtime_c/primitives_sequence_functions.h>
#include <rosidl_runtime_c/sequence_bound.h>
#include <rosidl_runtime_c/service_type_support_struct.h>
#include <rosidl_runtime_c/string_functions.h>
#include <rosidl_runtime_c/u16string_functions.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <rcutils/allocator.h>

// LLVMFuzzerTestOneInput function signature
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early return if no data or insufficient size for basic operations
    if (!data || size < 2) {
        return 0;
    }

    // Initialize variables
    rosidl_runtime_c__String__Sequence seq1 = {0};
    rosidl_runtime_c__String__Sequence seq2 = {0};
    rosidl_runtime_c__String single_string = {0};
    bool success = false;
    
    // Use first byte to determine sequence size (modulo to keep it reasonable)
    size_t seq_size = (data[0] % 16) + 1;  // Range: 1-16
    
    // 1. Test rosidl_runtime_c__String__Sequence__init
    success = rosidl_runtime_c__String__Sequence__init(&seq1, seq_size);
    if (!success) {
        // Initialization failed, clean up and return
        goto cleanup;
    }
    
    // 2. Test rosidl_runtime_c__String__init on a single string
    success = rosidl_runtime_c__String__init(&single_string);
    if (!success) {
        goto cleanup;
    }
    
    // Populate the sequence with data from fuzz input
    size_t data_offset = 1;  // Start after the first byte used for seq_size
    for (size_t i = 0; i < seq1.size && data_offset < size; i++) {
        // Calculate string length for this element (bounded by remaining data)
        size_t max_str_len = size - data_offset;
        if (max_str_len == 0) break;
        
        // Use a byte to determine string length (modulo to keep it reasonable)
        size_t str_len = (data[data_offset] % 32) + 1;  // Range: 1-32
        if (str_len > max_str_len) {
            str_len = max_str_len;
        }
        
        // Ensure we have enough data
        if (data_offset + str_len > size) {
            str_len = size - data_offset;
        }
        
        // Reallocate string buffer if needed
        if (seq1.data[i].capacity < str_len + 1) {
            rcutils_allocator_t allocator = rcutils_get_default_allocator();
            char *new_data = allocator.reallocate(
                seq1.data[i].data, 
                str_len + 1, 
                allocator.state
            );
            if (!new_data) {
                // Reallocation failed, skip this string
                continue;
            }
            seq1.data[i].data = new_data;
            seq1.data[i].capacity = str_len + 1;
        }
        
        // Copy data into string (safe copy with bounds checking)
        memcpy(seq1.data[i].data, data + data_offset, str_len);
        seq1.data[i].data[str_len] = '\0';
        seq1.data[i].size = str_len;
        
        data_offset += str_len;
    }
    
    // 3. Test rosidl_runtime_c__String__Sequence__copy
    // Initialize destination sequence with different size to test reallocation
    size_t seq2_initial_size = (seq_size > 1) ? seq_size / 2 : 0;
    success = rosidl_runtime_c__String__Sequence__init(&seq2, seq2_initial_size);
    if (!success) {
        goto cleanup;
    }
    
    // Perform the copy
    success = rosidl_runtime_c__String__Sequence__copy(&seq1, &seq2);
    if (!success) {
        // Copy failed, but we still need to clean up
        goto cleanup;
    }
    
    // Verify copy was successful by checking sizes match
    if (seq1.size != seq2.size) {
        // This shouldn't happen if copy succeeded, but handle gracefully
        goto cleanup;
    }
    
    // 4. Test rosidl_runtime_c__String__fini on the single string
    rosidl_runtime_c__String__fini(&single_string);
    
    // Reset single_string to avoid double-free
    single_string.data = NULL;
    single_string.size = 0;
    single_string.capacity = 0;

cleanup:
    // 5. Clean up all allocated resources
    
    // Finalize sequence 2 if it was initialized
    if (seq2.data != NULL || seq2.size > 0 || seq2.capacity > 0) {
        rosidl_runtime_c__String__Sequence__fini(&seq2);
    }
    
    // Finalize sequence 1 if it was initialized
    if (seq1.data != NULL || seq1.size > 0 || seq1.capacity > 0) {
        rosidl_runtime_c__String__Sequence__fini(&seq1);
    }
    
    // Finalize single string if it wasn't already finalized
    if (single_string.data != NULL || single_string.size > 0 || single_string.capacity > 0) {
        rosidl_runtime_c__String__fini(&single_string);
    }
    
    return 0;
}

// Main function for standalone testing (not used during fuzzing)
#ifdef STANDALONE_TEST
#include <stdio.h>
int main() {
    // Test with sample data
    uint8_t test_data[] = {5, 'H', 'e', 'l', 'l', 'o', 'W', 'o', 'r', 'l', 'd'};
    size_t test_size = sizeof(test_data);
    
    printf("Running standalone test...\n");
    int result = LLVMFuzzerTestOneInput(test_data, test_size);
    printf("Test completed with result: %d\n", result);
    
    return 0;
}
#endif
