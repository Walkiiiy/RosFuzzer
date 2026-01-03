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

// Forward declarations for types not fully defined in headers
typedef struct rcutils_allocator_s rcutils_allocator_t;
rcutils_allocator_t rcutils_get_default_allocator(void);
bool rosidl_runtime_c__String__assignn(rosidl_runtime_c__String * str, const char * data, size_t data_length);

// LLVMFuzzerTestOneInput function
#ifdef __cplusplus
extern "C" {
#endif

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early return if input is too small for meaningful testing
    if (size < 2) {
        return 0;
    }

    // Initialize variables
    rosidl_runtime_c__String__Sequence input_seq = {0};
    rosidl_runtime_c__String__Sequence output_seq = {0};
    bool success = false;
    
    // Determine sequence size from fuzz input (limit to reasonable size)
    size_t seq_size = (size_t)data[0] % 16;  // 0-15 elements
    if (seq_size == 0) seq_size = 1;  // Ensure at least one element
    
    // Allocate and initialize input sequence
    input_seq.capacity = seq_size;
    input_seq.size = seq_size;
    input_seq.data = (rosidl_runtime_c__String*)malloc(seq_size * sizeof(rosidl_runtime_c__String));
    if (!input_seq.data) {
        goto cleanup;
    }
    
    // Initialize all strings in input sequence
    for (size_t i = 0; i < seq_size; ++i) {
        if (!rosidl_runtime_c__String__init(&input_seq.data[i])) {
            // Clean up already initialized strings
            for (size_t j = 0; j < i; ++j) {
                rosidl_runtime_c__String__fini(&input_seq.data[j]);
            }
            free(input_seq.data);
            input_seq.data = NULL;
            goto cleanup;
        }
    }
    
    // Fill input strings with data from fuzz input
    size_t data_offset = 1;  // Skip first byte used for seq_size
    for (size_t i = 0; i < seq_size && data_offset < size; ++i) {
        // Determine string length from fuzz input
        size_t str_len = (size_t)data[data_offset % size] % 32;  // 0-31 characters
        if (str_len > size - data_offset - 1) {
            str_len = size - data_offset - 1;
        }
        if (str_len > 0) {
            // Use rosidl_runtime_c__String__assignn to copy data
            if (!rosidl_runtime_c__String__assignn(&input_seq.data[i], 
                                                  (const char*)&data[data_offset + 1], 
                                                  str_len)) {
                // If assignment fails, continue with next string
                // The string remains initialized but empty
            }
            data_offset += str_len + 1;
        } else {
            data_offset++;
        }
    }
    
    // Initialize output sequence with smaller capacity to test reallocation
    output_seq.capacity = seq_size / 2;
    if (output_seq.capacity == 0) output_seq.capacity = 1;
    output_seq.size = 0;
    output_seq.data = (rosidl_runtime_c__String*)malloc(output_seq.capacity * sizeof(rosidl_runtime_c__String));
    if (!output_seq.data) {
        goto cleanup;
    }
    
    // Initialize strings in output sequence
    for (size_t i = 0; i < output_seq.capacity; ++i) {
        if (!rosidl_runtime_c__String__init(&output_seq.data[i])) {
            // Clean up already initialized strings
            for (size_t j = 0; j < i; ++j) {
                rosidl_runtime_c__String__fini(&output_seq.data[j]);
            }
            free(output_seq.data);
            output_seq.data = NULL;
            goto cleanup;
        }
    }
    
    // Test rosidl_runtime_c__String__Sequence__copy
    success = rosidl_runtime_c__String__Sequence__copy(&input_seq, &output_seq);
    if (!success) {
        // Copy failed, but we continue to test other functions
    }
    
    // Test rosidl_runtime_c__String__are_equal on some pairs
    if (seq_size >= 2 && output_seq.size >= 2) {
        bool equal = rosidl_runtime_c__String__are_equal(&input_seq.data[0], &output_seq.data[0]);
        // Also test with same string
        equal = rosidl_runtime_c__String__are_equal(&input_seq.data[0], &input_seq.data[0]);
        
        // Test with potentially different strings
        if (seq_size > 1) {
            equal = rosidl_runtime_c__String__are_equal(&input_seq.data[0], &input_seq.data[1]);
        }
    }
    
    // Test rosidl_runtime_c__String__copy on individual strings
    if (seq_size >= 1) {
        rosidl_runtime_c__String temp_str;
        if (rosidl_runtime_c__String__init(&temp_str)) {
            success = rosidl_runtime_c__String__copy(&input_seq.data[0], &temp_str);
            rosidl_runtime_c__String__fini(&temp_str);
        }
    }
    
    success = true;

cleanup:
    // Clean up input sequence
    if (input_seq.data) {
        for (size_t i = 0; i < input_seq.size; ++i) {
            rosidl_runtime_c__String__fini(&input_seq.data[i]);
        }
        free(input_seq.data);
    }
    
    // Clean up output sequence
    if (output_seq.data) {
        // Only finalize strings that were initialized
        size_t strings_to_fini = (output_seq.capacity < output_seq.size) ? 
                                 output_seq.capacity : output_seq.size;
        for (size_t i = 0; i < strings_to_fini; ++i) {
            rosidl_runtime_c__String__fini(&output_seq.data[i]);
        }
        free(output_seq.data);
    }
    
    return 0;
}

#ifdef __cplusplus
}
#endif
