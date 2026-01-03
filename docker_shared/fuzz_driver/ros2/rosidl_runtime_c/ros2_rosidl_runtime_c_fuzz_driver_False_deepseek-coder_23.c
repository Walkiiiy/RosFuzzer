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

// Forward declarations for functions used in the API but not provided in source
bool rosidl_runtime_c__U16String__init(rosidl_runtime_c__U16String *str);
bool rosidl_runtime_c__U16String__copy(
    const rosidl_runtime_c__U16String *input,
    rosidl_runtime_c__U16String *output);
bool rosidl_runtime_c__U16String__are_equal(
    const rosidl_runtime_c__U16String *lhs,
    const rosidl_runtime_c__U16String *rhs);

// LLVMFuzzerTestOneInput function
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early return if no data
    if (data == NULL || size == 0) {
        return 0;
    }

    // Initialize sequences
    rosidl_runtime_c__U16String__Sequence seq1 = {0};
    rosidl_runtime_c__U16String__Sequence seq2 = {0};
    
    // Determine sequence size from fuzz input (limit to reasonable size)
    size_t seq_size = size % 16;  // Limit to 0-15 elements to avoid excessive memory usage
    
    // Initialize first sequence
    if (!rosidl_runtime_c__U16String__Sequence__init(&seq1, seq_size)) {
        // Initialization failed, nothing to clean up
        return 0;
    }
    
    // Initialize second sequence with same size
    if (!rosidl_runtime_c__U16String__Sequence__init(&seq2, seq_size)) {
        // Clean up seq1 before returning
        rosidl_runtime_c__U16String__Sequence__fini(&seq1);
        return 0;
    }
    
    // Fill sequences with data from fuzz input
    size_t data_index = 0;
    for (size_t i = 0; i < seq_size; i++) {
        // Determine string length for this element (1-16 characters)
        size_t str_len = (data[data_index % size] % 16) + 1;
        data_index = (data_index + 1) % size;
        
        // Allocate buffer for UTF-16 string
        uint16_t *str_data = malloc(str_len * sizeof(uint16_t));
        if (str_data == NULL) {
            // Clean up and return on allocation failure
            rosidl_runtime_c__U16String__Sequence__fini(&seq1);
            rosidl_runtime_c__U16String__Sequence__fini(&seq2);
            return 0;
        }
        
        // Fill string with data from fuzz input
        for (size_t j = 0; j < str_len; j++) {
            if (data_index < size) {
                str_data[j] = (uint16_t)data[data_index];
                data_index = (data_index + 1) % size;
            } else {
                str_data[j] = 0;
            }
        }
        
        // Free existing data if any (should be empty after init)
        if (seq1.data[i].data) {
            free(seq1.data[i].data);
        }
        
        // Assign to sequence element
        seq1.data[i].data = str_data;
        seq1.data[i].size = str_len;
        seq1.data[i].capacity = str_len;
        
        // Copy to second sequence
        uint16_t *str_data2 = malloc(str_len * sizeof(uint16_t));
        if (str_data2 == NULL) {
            // Clean up and return on allocation failure
            rosidl_runtime_c__U16String__Sequence__fini(&seq1);
            rosidl_runtime_c__U16String__Sequence__fini(&seq2);
            return 0;
        }
        
        memcpy(str_data2, str_data, str_len * sizeof(uint16_t));
        if (seq2.data[i].data) {
            free(seq2.data[i].data);
        }
        seq2.data[i].data = str_data2;
        seq2.data[i].size = str_len;
        seq2.data[i].capacity = str_len;
    }
    
    // Test rosidl_runtime_c__U16String__Sequence__are_equal
    // Sequences should be equal since we copied the data
    bool are_equal = rosidl_runtime_c__U16String__Sequence__are_equal(&seq1, &seq2);
    (void)are_equal;  // Mark as used to avoid compiler warning
    
    // Create a third sequence for copy testing
    rosidl_runtime_c__U16String__Sequence seq3 = {0};
    
    // Initialize seq3 with different size
    size_t seq3_size = (seq_size + 1) % 16;  // Different size
    if (!rosidl_runtime_c__U16String__Sequence__init(&seq3, seq3_size)) {
        rosidl_runtime_c__U16String__Sequence__fini(&seq1);
        rosidl_runtime_c__U16String__Sequence__fini(&seq2);
        return 0;
    }
    
    // Test rosidl_runtime_c__U16String__Sequence__copy
    // Copy seq1 to seq3 (seq3 should be resized)
    bool copy_success = rosidl_runtime_c__U16String__Sequence__copy(&seq1, &seq3);
    (void)copy_success;  // Mark as used
    
    // Test equality again after copy
    are_equal = rosidl_runtime_c__U16String__Sequence__are_equal(&seq1, &seq3);
    (void)are_equal;
    
    // Clean up all sequences
    rosidl_runtime_c__U16String__Sequence__fini(&seq1);
    rosidl_runtime_c__U16String__Sequence__fini(&seq2);
    rosidl_runtime_c__U16String__Sequence__fini(&seq3);
    
    return 0;
}

// Stub implementations for missing functions
// These are simplified versions that match the expected behavior
bool rosidl_runtime_c__U16String__init(rosidl_runtime_c__U16String *str) {
    if (!str) return false;
    str->data = NULL;
    str->size = 0;
    str->capacity = 0;
    return true;
}

bool rosidl_runtime_c__U16String__copy(
    const rosidl_runtime_c__U16String *input,
    rosidl_runtime_c__U16String *output) {
    if (!input || !output) return false;
    
    // Allocate new buffer
    uint16_t *new_data = malloc(input->size * sizeof(uint16_t));
    if (!new_data) return false;
    
    // Copy data
    memcpy(new_data, input->data, input->size * sizeof(uint16_t));
    
    // Free old data if any
    if (output->data) {
        free(output->data);
    }
    
    output->data = new_data;
    output->size = input->size;
    output->capacity = input->size;
    return true;
}

bool rosidl_runtime_c__U16String__are_equal(
    const rosidl_runtime_c__U16String *lhs,
    const rosidl_runtime_c__U16String *rhs) {
    if (!lhs || !rhs) return false;
    if (lhs->size != rhs->size) return false;
    return memcmp(lhs->data, rhs->data, lhs->size * sizeof(uint16_t)) == 0;
}
