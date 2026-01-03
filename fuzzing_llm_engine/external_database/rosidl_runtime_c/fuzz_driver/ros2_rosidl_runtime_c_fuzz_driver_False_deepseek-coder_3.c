#include <rosidl_runtime_c/message_type_support.h>
#include <rosidl_runtime_c/primitives_sequence_functions.h>
#include <rosidl_runtime_c/sequence_bound.h>
#include <rosidl_runtime_c/service_type_support.h>
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
    // Early return if insufficient data
    if (size < 2) {
        return 0;
    }

    // Initialize variables
    rosidl_runtime_c__U16String__Sequence seq1 = {0};
    rosidl_runtime_c__U16String__Sequence seq2 = {0};
    rosidl_runtime_c__Sequence__bound bound_handle = {0};
    const rosidl_runtime_c__Sequence__bound *result_handle = NULL;
    bool init_success = false;
    bool are_equal = false;

    // Step 1: Use get_sequence_bound_handle and get_sequence_bound_handle_function
    // We need to set up a minimal bound handle structure for testing
    // Use fuzz data to create identifier string (with bounds checking)
    size_t identifier_len = size > 100 ? 100 : size;
    char *identifier = (char *)malloc(identifier_len + 1);
    if (!identifier) {
        return 0;
    }
    
    // Copy data for identifier, ensuring null termination
    memcpy(identifier, data, identifier_len);
    identifier[identifier_len] = '\0';
    
    // Set up a dummy bound handle for testing
    bound_handle.func = (void (*)(void))get_sequence_bound_handle_function;
    bound_handle.typesupport_identifier = "test_identifier";
    
    // Call get_sequence_bound_handle
    result_handle = get_sequence_bound_handle(&bound_handle, identifier);
    // Note: result_handle will be NULL since identifiers don't match
    
    // Step 2: Initialize U16String sequences using fuzz data
    // Determine sequence size from fuzz input (ensure it's reasonable)
    size_t seq_size = (size_t)(data[0] % 8) + 1;  // 1-8 elements
    
    // Initialize first sequence
    init_success = rosidl_runtime_c__U16String__Sequence__init(&seq1, seq_size);
    if (!init_success) {
        free(identifier);
        return 0;
    }
    
    // Initialize second sequence with same size
    init_success = rosidl_runtime_c__U16String__Sequence__init(&seq2, seq_size);
    if (!init_success) {
        rosidl_runtime_c__U16String__Sequence__fini(&seq1);
        free(identifier);
        return 0;
    }
    
    // Step 3: Populate sequences with fuzz data
    size_t data_offset = 1;  // Start after the first byte used for seq_size
    for (size_t i = 0; i < seq_size; i++) {
        if (data_offset >= size) {
            break;  // Not enough fuzz data
        }
        
        // Determine string length for this element (limited to avoid overflow)
        size_t str_len = (size_t)(data[data_offset] % 16) + 1;  // 1-16 characters
        data_offset++;
        
        // Ensure we have enough data
        if (data_offset + str_len * 2 > size) {
            str_len = (size - data_offset) / 2;
            if (str_len == 0) {
                break;
            }
        }
        
        // Resize the U16String to hold our data
        if (!rosidl_runtime_c__U16String__resize(&seq1.data[i], str_len)) {
            // Cleanup and exit if resize fails
            for (size_t j = 0; j <= i; j++) {
                rosidl_runtime_c__U16String__fini(&seq1.data[j]);
                rosidl_runtime_c__U16String__fini(&seq2.data[j]);
            }
            rosidl_runtime_c__U16String__Sequence__fini(&seq1);
            rosidl_runtime_c__U16String__Sequence__fini(&seq2);
            free(identifier);
            return 0;
        }
        
        // Copy fuzz data into the U16String
        for (size_t j = 0; j < str_len && data_offset < size; j++) {
            uint16_t char_val = 0;
            if (data_offset + 1 < size) {
                char_val = (data[data_offset] << 8) | data[data_offset + 1];
                data_offset += 2;
            } else {
                char_val = data[data_offset];
                data_offset++;
            }
            seq1.data[i].data[j] = char_val;
        }
        
        // For testing equality, make seq2 identical to seq1
        if (!rosidl_runtime_c__U16String__resize(&seq2.data[i], str_len)) {
            // Cleanup and exit if resize fails
            for (size_t j = 0; j <= i; j++) {
                rosidl_runtime_c__U16String__fini(&seq1.data[j]);
                rosidl_runtime_c__U16String__fini(&seq2.data[j]);
            }
            rosidl_runtime_c__U16String__Sequence__fini(&seq1);
            rosidl_runtime_c__U16String__Sequence__fini(&seq2);
            free(identifier);
            return 0;
        }
        
        memcpy(seq2.data[i].data, seq1.data[i].data, str_len * sizeof(uint16_t));
    }
    
    // Step 4: Test rosidl_runtime_c__U16String__are_equal on individual strings
    if (seq_size > 0) {
        bool string_equal = rosidl_runtime_c__U16String__are_equal(
            &seq1.data[0], &seq2.data[0]);
        // Result should be true since we made them identical
        (void)string_equal;  // Mark as used to avoid compiler warning
    }
    
    // Step 5: Test rosidl_runtime_c__U16String__Sequence__are_equal
    are_equal = rosidl_runtime_c__U16String__Sequence__are_equal(&seq1, &seq2);
    // Result should be true since sequences are identical
    
    // Also test with NULL pointers (error cases)
    bool null_test1 = rosidl_runtime_c__U16String__Sequence__are_equal(NULL, &seq1);
    bool null_test2 = rosidl_runtime_c__U16String__Sequence__are_equal(&seq1, NULL);
    bool null_test3 = rosidl_runtime_c__U16String__Sequence__are_equal(NULL, NULL);
    (void)null_test1;
    (void)null_test2;
    (void)null_test3;
    
    // Test individual string equality with NULL pointers
    if (seq_size > 0) {
        bool null_str_test1 = rosidl_runtime_c__U16String__are_equal(NULL, &seq1.data[0]);
        bool null_str_test2 = rosidl_runtime_c__U16String__are_equal(&seq1.data[0], NULL);
        bool null_str_test3 = rosidl_runtime_c__U16String__are_equal(NULL, NULL);
        (void)null_str_test1;
        (void)null_str_test2;
        (void)null_str_test3;
    }
    
    // Cleanup
    rosidl_runtime_c__U16String__Sequence__fini(&seq1);
    rosidl_runtime_c__U16String__Sequence__fini(&seq2);
    free(identifier);
    
    return 0;
}
