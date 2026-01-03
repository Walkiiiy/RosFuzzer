#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

// ROS 2 headers
#include "rosidl_runtime_c/message_type_support.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"
#include "rosidl_runtime_c/sequence_bound.h"
#include "rosidl_runtime_c/service_type_support.h"
#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_runtime_c/u16string_functions.h"

// Note: <sstream> is a C++ header, but we're writing C code.
// We'll use C string functions instead.

// Forward declarations for functions we need from the API
const rosidl_runtime_c__Sequence__bound * get_sequence_bound_handle(
    const rosidl_runtime_c__Sequence__bound * handle, const char * identifier);

bool rosidl_runtime_c__U16String__Sequence__are_equal(
    const rosidl_runtime_c__U16String__Sequence * lhs,
    const rosidl_runtime_c__U16String__Sequence * rhs);

bool rosidl_runtime_c__U16String__Sequence__init(
    rosidl_runtime_c__U16String__Sequence * sequence, size_t size);

const rosidl_runtime_c__Sequence__bound * get_sequence_bound_handle_function(
    const rosidl_runtime_c__Sequence__bound * handle, const char * identifier);

bool rosidl_runtime_c__U16String__are_equal(
    const rosidl_runtime_c__U16String * lhs,
    const rosidl_runtime_c__U16String * rhs);

// Fuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early return if input is too small for basic operations
    if (size < 2) {
        return 0;
    }

    // Initialize variables
    rosidl_runtime_c__U16String__Sequence seq1 = {0};
    rosidl_runtime_c__U16String__Sequence seq2 = {0};
    rosidl_runtime_c__Sequence__bound bound_handle = {0};
    const rosidl_runtime_c__Sequence__bound *result_handle = NULL;
    
    // Use fuzz input to determine sequence sizes
    // Use first byte for seq1 size (modulo to keep it reasonable)
    size_t seq1_size = (data[0] % 16) + 1;  // 1-16 elements
    
    // Use second byte for seq2 size
    size_t seq2_size = (data[1] % 16) + 1;  // 1-16 elements
    
    // Initialize first sequence
    if (!rosidl_runtime_c__U16String__Sequence__init(&seq1, seq1_size)) {
        // Initialization failed, clean up and return
        goto cleanup;
    }
    
    // Initialize second sequence
    if (!rosidl_runtime_c__U16String__Sequence__init(&seq2, seq2_size)) {
        // Initialization failed, clean up seq1 and return
        rosidl_runtime_c__U16String__Sequence__fini(&seq1);
        goto cleanup;
    }
    
    // Populate sequences with data from fuzz input
    size_t data_offset = 2;
    for (size_t i = 0; i < seq1_size && data_offset + 1 < size; i++) {
        // Determine string length for this element (1-8 uint16_t characters)
        size_t str_len = (data[data_offset] % 8) + 1;
        data_offset++;
        
        // Ensure we have enough data
        if (data_offset + str_len * 2 > size) {
            break;
        }
        
        // Resize the U16String to hold our data
        if (!rosidl_runtime_c__U16String__resize(&seq1.data[i], str_len)) {
            // Resize failed, continue with next element
            continue;
        }
        
        // Copy data from fuzz input
        for (size_t j = 0; j < str_len && data_offset + 1 < size; j++) {
            // Create a uint16_t from two bytes
            uint16_t val = (data[data_offset] << 8) | data[data_offset + 1];
            seq1.data[i].data[j] = val;
            data_offset += 2;
        }
    }
    
    // Reset offset for second sequence
    data_offset = 2;
    for (size_t i = 0; i < seq2_size && data_offset + 1 < size; i++) {
        // Determine string length for this element (1-8 uint16_t characters)
        size_t str_len = (data[data_offset] % 8) + 1;
        data_offset++;
        
        // Ensure we have enough data
        if (data_offset + str_len * 2 > size) {
            break;
        }
        
        // Resize the U16String to hold our data
        if (!rosidl_runtime_c__U16String__resize(&seq2.data[i], str_len)) {
            // Resize failed, continue with next element
            continue;
        }
        
        // Copy data from fuzz input
        for (size_t j = 0; j < str_len && data_offset + 1 < size; j++) {
            // Create a uint16_t from two bytes
            uint16_t val = (data[data_offset] << 8) | data[data_offset + 1];
            seq2.data[i].data[j] = val;
            data_offset += 2;
        }
    }
    
    // Test rosidl_runtime_c__U16String__are_equal on some elements
    if (seq1_size > 0 && seq2_size > 0) {
        // Compare first elements of each sequence
        bool strings_equal = rosidl_runtime_c__U16String__are_equal(
            &seq1.data[0], &seq2.data[0]);
        (void)strings_equal;  // Use result to avoid unused variable warning
    }
    
    // Test rosidl_runtime_c__U16String__Sequence__are_equal
    bool sequences_equal = rosidl_runtime_c__U16String__Sequence__are_equal(&seq1, &seq2);
    (void)sequences_equal;  // Use result to avoid unused variable warning
    
    // Test get_sequence_bound_handle_function
    // First, set up a mock bound handle
    bound_handle.typesupport_identifier = "test_identifier";
    bound_handle.func = (void*)get_sequence_bound_handle_function;
    
    // Test with matching identifier
    result_handle = get_sequence_bound_handle_function(&bound_handle, "test_identifier");
    (void)result_handle;  // Use result to avoid unused variable warning
    
    // Test with non-matching identifier
    result_handle = get_sequence_bound_handle_function(&bound_handle, "wrong_identifier");
    (void)result_handle;  // Use result to avoid unused variable warning
    
    // Test get_sequence_bound_handle
    // This will call the function pointer in bound_handle
    result_handle = get_sequence_bound_handle(&bound_handle, "test_identifier");
    (void)result_handle;  // Use result to avoid unused variable warning
    
    // Test with NULL handle (should trigger assert in debug builds)
    // We'll skip this in fuzzing to avoid crashes from assertions
    // result_handle = get_sequence_bound_handle(NULL, "test_identifier");
    
cleanup:
    // Clean up all allocated resources
    if (seq1.data) {
        rosidl_runtime_c__U16String__Sequence__fini(&seq1);
    }
    if (seq2.data) {
        rosidl_runtime_c__U16String__Sequence__fini(&seq2);
    }
    
    return 0;
}

// Note: We need to provide implementations for the missing functions
// that are referenced but not defined in the API source code

// Implementation of rosidl_runtime_c__U16String__resize
bool rosidl_runtime_c__U16String__resize(rosidl_runtime_c__U16String * str, size_t size) {
    if (!str) {
        return false;
    }
    
    if (size == str->size) {
        return true;
    }
    
    if (size == 0) {
        free(str->data);
        str->data = NULL;
        str->size = 0;
        str->capacity = 0;
        return true;
    }
    
    uint16_t * new_data = (uint16_t *)realloc(str->data, size * sizeof(uint16_t));
    if (!new_data) {
        return false;
    }
    
    // Initialize new memory if expanding
    if (size > str->size) {
        memset(new_data + str->size, 0, (size - str->size) * sizeof(uint16_t));
    }
    
    str->data = new_data;
    str->size = size;
    str->capacity = size;
    return true;
}

// Implementation of rosidl_runtime_c__U16String__Sequence__fini
void rosidl_runtime_c__U16String__Sequence__fini(rosidl_runtime_c__U16String__Sequence * sequence) {
    if (!sequence) {
        return;
    }
    
    if (sequence->data) {
        // Finalize each string in the sequence
        for (size_t i = 0; i < sequence->size; ++i) {
            rosidl_runtime_c__U16String__fini(&sequence->data[i]);
        }
        
        // Free the array
        free(sequence->data);
        sequence->data = NULL;
    }
    
    sequence->size = 0;
    sequence->capacity = 0;
}

// Implementation of rosidl_runtime_c__U16String__fini
void rosidl_runtime_c__U16String__fini(rosidl_runtime_c__U16String * str) {
    if (!str) {
        return;
    }
    
    if (str->data) {
        free(str->data);
        str->data = NULL;
    }
    
    str->size = 0;
    str->capacity = 0;
}

// Implementation of rcutils_get_default_allocator (simplified)
typedef struct rcutils_allocator_s {
    void * (*allocate)(size_t size, void * state);
    void (*deallocate)(void * pointer, void * state);
    void * state;
} rcutils_allocator_t;

rcutils_allocator_t rcutils_get_default_allocator(void) {
    rcutils_allocator_t allocator = {
        .allocate = malloc,
        .deallocate = free,
        .state = NULL
    };
    return allocator;
}

// Macro definition for RCUTILS_CAN_RETURN_WITH_ERROR_OF
#define RCUTILS_CAN_RETURN_WITH_ERROR_OF(x)

// Note: The actual implementations of these functions would normally come from
// the ROS 2 libraries. These are simplified versions for the fuzzer to compile.
