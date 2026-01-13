#include <rosidl_runtime_c/action_type_support_struct.h>
#include <rosidl_runtime_c/message_initialization.h>
#include <rosidl_runtime_c/message_type_support_struct.h>
#include <rosidl_runtime_c/primitives_sequence_functions.h>
#include <rosidl_runtime_c/primitives_sequence.h>
#include <rosidl_runtime_c/sequence_bound.h>
#include <rosidl_runtime_c/service_type_support_struct.h>
#include <rosidl_runtime_c/string_functions.h>
#include <rosidl_runtime_c/u16string_functions.h>
#include <rosidl_runtime_c/visibility_control.h>
#include <rosidl_runtime_c/string.h>
#include <rosidl_runtime_c/string_bound.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <rcutils/allocator.h>

// Fuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early return if input is too small for meaningful fuzzing
    if (size < 4) {
        return 0;
    }

    // Initialize default allocator (used in the fallback implementation if needed)
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    (void)allocator;  // Mark as used to avoid warning
    
    // Use first byte to determine sequence sizes (with bounds checking)
    uint8_t size_mod = data[0];
    size_t seq_size = (size_mod % 8) + 1;  // Sequence size between 1-8
    
    // Initialize two sequences for comparison
    rosidl_runtime_c__U16String__Sequence seq1;
    rosidl_runtime_c__U16String__Sequence seq2;
    
    // Initialize sequences to zero to avoid undefined behavior
    memset(&seq1, 0, sizeof(seq1));
    memset(&seq2, 0, sizeof(seq2));
    
    // Initialize first sequence
    if (!rosidl_runtime_c__U16String__Sequence__init(&seq1, seq_size)) {
        // If initialization fails, clean up and return
        rosidl_runtime_c__U16String__Sequence__fini(&seq1);
        rosidl_runtime_c__U16String__Sequence__fini(&seq2);
        return 0;
    }
    
    // Initialize second sequence
    if (!rosidl_runtime_c__U16String__Sequence__init(&seq2, seq_size)) {
        // Clean up and return if second sequence fails
        rosidl_runtime_c__U16String__Sequence__fini(&seq1);
        rosidl_runtime_c__U16String__Sequence__fini(&seq2);
        return 0;
    }
    
    // Fill sequences with data from fuzz input
    size_t data_offset = 1;  // Skip the size byte
    for (size_t i = 0; i < seq_size; i++) {
        // Calculate available data for this string
        size_t remaining_data = (size > data_offset) ? (size - data_offset) : 0;
        
        // Determine string length (limited to avoid excessive allocation)
        size_t str_len = 0;
        if (remaining_data > 0) {
            // Use next byte modulo 64 to limit string length
            str_len = (data[data_offset % size] % 64) + 1;
            data_offset = (data_offset + 1) % size;
            
            // Recalculate remaining data after consuming the length byte
            remaining_data = (size > data_offset) ? (size - data_offset) : 0;
        }
        
        // Ensure we don't exceed available data for uint16_t array
        size_t max_str_len = remaining_data / sizeof(uint16_t);
        if (str_len > max_str_len) {
            str_len = max_str_len;
        }
        
        // Only resize if we have data to copy
        if (str_len > 0) {
            // Resize strings to accommodate data
            if (!rosidl_runtime_c__U16String__resize(&seq1.data[i], str_len)) {
                // If resize fails, clean up and return
                rosidl_runtime_c__U16String__Sequence__fini(&seq1);
                rosidl_runtime_c__U16String__Sequence__fini(&seq2);
                return 0;
            }
            
            if (!rosidl_runtime_c__U16String__resize(&seq2.data[i], str_len)) {
                rosidl_runtime_c__U16String__Sequence__fini(&seq1);
                rosidl_runtime_c__U16String__Sequence__fini(&seq2);
                return 0;
            }
            
            // Copy data to both strings (same content for equality test)
            if (str_len > 0 && remaining_data >= str_len * sizeof(uint16_t)) {
                memcpy(seq1.data[i].data, &data[data_offset], str_len * sizeof(uint16_t));
                memcpy(seq2.data[i].data, &data[data_offset], str_len * sizeof(uint16_t));
                data_offset += str_len * sizeof(uint16_t);
            }
        } else {
            // Initialize empty strings
            if (!rosidl_runtime_c__U16String__resize(&seq1.data[i], 0)) {
                rosidl_runtime_c__U16String__Sequence__fini(&seq1);
                rosidl_runtime_c__U16String__Sequence__fini(&seq2);
                return 0;
            }
            if (!rosidl_runtime_c__U16String__resize(&seq2.data[i], 0)) {
                rosidl_runtime_c__U16String__Sequence__fini(&seq1);
                rosidl_runtime_c__U16String__Sequence__fini(&seq2);
                return 0;
            }
        }
    }
    
    // Test sequence equality - should return true since we filled them identically
    bool sequences_equal = rosidl_runtime_c__U16String__Sequence__are_equal(&seq1, &seq2);
    (void)sequences_equal;  // Mark as used to avoid compiler warnings
    
    // Test individual string equality for each pair
    for (size_t i = 0; i < seq_size; i++) {
        bool strings_equal = rosidl_runtime_c__U16String__are_equal(&seq1.data[i], &seq2.data[i]);
        (void)strings_equal;  // Mark as used
    }
    
    // Test with NULL pointers for robustness
    bool null_test1 = rosidl_runtime_c__U16String__Sequence__are_equal(NULL, &seq1);
    bool null_test2 = rosidl_runtime_c__U16String__Sequence__are_equal(&seq1, NULL);
    bool null_test3 = rosidl_runtime_c__U16String__Sequence__are_equal(NULL, NULL);
    (void)null_test1;
    (void)null_test2;
    (void)null_test3;
    
    // Test individual string comparison with NULL
    bool str_null_test1 = rosidl_runtime_c__U16String__are_equal(NULL, &seq1.data[0]);
    bool str_null_test2 = rosidl_runtime_c__U16String__are_equal(&seq1.data[0], NULL);
    bool str_null_test3 = rosidl_runtime_c__U16String__are_equal(NULL, NULL);
    (void)str_null_test1;
    (void)str_null_test2;
    (void)str_null_test3;
    
    // Create a mock sequence bound handle for testing
    rosidl_runtime_c__Sequence__bound mock_handle;
    memset(&mock_handle, 0, sizeof(mock_handle));
    
    // Set up a typesupport identifier
    const char* test_identifier = "test_type";
    mock_handle.typesupport_identifier = test_identifier;
    
    // Test get_sequence_bound_handle_function
    const rosidl_runtime_c__Sequence__bound* result1 = 
        get_sequence_bound_handle_function(&mock_handle, test_identifier);
    (void)result1;
    
    // Test with different identifier
    const rosidl_runtime_c__Sequence__bound* result2 = 
        get_sequence_bound_handle_function(&mock_handle, "different_type");
    (void)result2;
    
    // Test get_sequence_bound_handle with a function pointer
    // Cast through uintptr_t to avoid direct void* to function pointer conversion
    mock_handle.func = (rosidl_runtime_c__bound_handle_function)(uintptr_t)get_sequence_bound_handle_function;
    const rosidl_runtime_c__Sequence__bound* result3 = 
        get_sequence_bound_handle(&mock_handle, test_identifier);
    (void)result3;
    
    // Test with NULL handle (should assert in debug, but we protect in release)
    #ifndef NDEBUG
    // In debug mode, these would assert - we skip them
    #else
    const rosidl_runtime_c__Sequence__bound* result4 = 
        get_sequence_bound_handle_function(NULL, test_identifier);
    const rosidl_runtime_c__Sequence__bound* result5 = 
        get_sequence_bound_handle(NULL, test_identifier);
    (void)result4;
    (void)result5;
    #endif
    
    // Clean up sequences
    rosidl_runtime_c__U16String__Sequence__fini(&seq1);
    rosidl_runtime_c__U16String__Sequence__fini(&seq2);
    
    return 0;
}

// Remove the static implementation since the function is already declared in the header
// The header u16string_functions.h already provides the declaration
