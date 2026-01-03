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

// Function to extract a size_t value from fuzz data
static size_t extract_size_t(const uint8_t **data, size_t *size) {
    if (*size < sizeof(size_t)) {
        return 0;
    }
    size_t value;
    memcpy(&value, *data, sizeof(size_t));
    *data += sizeof(size_t);
    *size -= sizeof(size_t);
    return value;
}

// Function to extract a uint16_t value from fuzz data
static uint16_t extract_uint16_t(const uint8_t **data, size_t *size) {
    if (*size < sizeof(uint16_t)) {
        return 0;
    }
    uint16_t value;
    memcpy(&value, *data, sizeof(uint16_t));
    *data += sizeof(uint16_t);
    *size -= sizeof(uint16_t);
    return value;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early exit if we don't have enough data for basic operations
    if (size < sizeof(size_t) * 3) {
        return 0;
    }

    // Extract parameters from fuzz data
    const uint8_t *current_data = data;
    size_t remaining_size = size;
    
    // Extract sizes for sequences
    size_t seq1_size = extract_size_t(&current_data, &remaining_size) % 10;  // Limit size
    size_t seq2_size = extract_size_t(&current_data, &remaining_size) % 10;  // Limit size
    
    // Extract size for string operations
    size_t str_size = extract_size_t(&current_data, &remaining_size) % 100;  // Limit size
    
    // Initialize two boolean sequences for comparison
    rosidl_runtime_c__boolean__Sequence bool_seq1;
    rosidl_runtime_c__boolean__Sequence bool_seq2;
    
    // Initialize the sequences with default values
    memset(&bool_seq1, 0, sizeof(rosidl_runtime_c__boolean__Sequence));
    memset(&bool_seq2, 0, sizeof(rosidl_runtime_c__boolean__Sequence));
    
    // Allocate and initialize boolean sequences
    if (!rosidl_runtime_c__boolean__Sequence__init(&bool_seq1, seq1_size)) {
        goto cleanup;
    }
    
    if (!rosidl_runtime_c__boolean__Sequence__init(&bool_seq2, seq2_size)) {
        rosidl_runtime_c__boolean__Sequence__fini(&bool_seq1);
        goto cleanup;
    }
    
    // Fill boolean sequences with data from fuzz input
    for (size_t i = 0; i < bool_seq1.size && remaining_size > 0; i++) {
        bool_seq1.data[i] = (extract_uint16_t(&current_data, &remaining_size) % 2) != 0;
    }
    
    for (size_t i = 0; i < bool_seq2.size && remaining_size > 0; i++) {
        bool_seq2.data[i] = (extract_uint16_t(&current_data, &remaining_size) % 2) != 0;
    }
    
    // Call rosidl_runtime_c__bool__Sequence__are_equal
    bool sequences_equal = rosidl_runtime_c__bool__Sequence__are_equal(&bool_seq1, &bool_seq2);
    (void)sequences_equal;  // Use result to avoid unused variable warning
    
    // Initialize U16String sequence
    rosidl_runtime_c__U16String__Sequence u16string_seq;
    memset(&u16string_seq, 0, sizeof(rosidl_runtime_c__U16String__Sequence));
    
    // Call rosidl_runtime_c__U16String__Sequence__init
    size_t u16seq_size = str_size % 5;  // Limit sequence size
    if (!rosidl_runtime_c__U16String__Sequence__init(&u16string_seq, u16seq_size)) {
        rosidl_runtime_c__boolean__Sequence__fini(&bool_seq1);
        rosidl_runtime_c__boolean__Sequence__fini(&bool_seq2);
        goto cleanup;
    }
    
    // Process each string in the sequence
    for (size_t i = 0; i < u16string_seq.size; i++) {
        rosidl_runtime_c__U16String *str = &u16string_seq.data[i];
        
        // Determine how much data we can use for this string
        size_t available_for_string = remaining_size / sizeof(uint16_t);
        if (available_for_string == 0) {
            break;
        }
        
        // Limit string size to prevent excessive memory usage
        size_t copy_size = available_for_string % 50;
        if (copy_size == 0) {
            copy_size = 1;
        }
        
        // Ensure we have enough data
        if (remaining_size < copy_size * sizeof(uint16_t)) {
            break;
        }
        
        // Create a temporary buffer for the string data
        uint16_t *temp_buffer = (uint16_t *)malloc((copy_size + 1) * sizeof(uint16_t));
        if (!temp_buffer) {
            break;
        }
        
        // Fill buffer with data from fuzz input
        for (size_t j = 0; j < copy_size && remaining_size >= sizeof(uint16_t); j++) {
            temp_buffer[j] = extract_uint16_t(&current_data, &remaining_size);
        }
        temp_buffer[copy_size] = 0;  // Null terminate
        
        // Call rosidl_runtime_c__U16String__assignn
        if (!rosidl_runtime_c__U16String__assignn(str, temp_buffer, copy_size)) {
            free(temp_buffer);
            break;
        }
        
        // Call rosidl_runtime_c__U16String__len
        size_t str_len = rosidl_runtime_c__U16String__len(str->data);
        (void)str_len;  // Use result to avoid unused variable warning
        
        // Call rosidl_runtime_c__U16String__resize
        // Resize to a smaller size to test the resize function
        size_t new_size = (copy_size > 2) ? (copy_size / 2) : 1;
        if (!rosidl_runtime_c__U16String__resize(str, new_size)) {
            free(temp_buffer);
            break;
        }
        
        free(temp_buffer);
    }
    
cleanup:
    // Clean up boolean sequences
    rosidl_runtime_c__boolean__Sequence__fini(&bool_seq1);
    rosidl_runtime_c__boolean__Sequence__fini(&bool_seq2);
    
    // Clean up U16String sequence
    if (u16string_seq.data) {
        for (size_t i = 0; i < u16string_seq.size; i++) {
            rosidl_runtime_c__U16String__fini(&u16string_seq.data[i]);
        }
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        allocator.deallocate(u16string_seq.data, allocator.state);
    }
    
    return 0;
}
