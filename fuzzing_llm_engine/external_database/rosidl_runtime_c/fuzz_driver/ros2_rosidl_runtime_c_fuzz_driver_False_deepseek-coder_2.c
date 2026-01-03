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

// We need rcutils headers for the allocator
#include <rcutils/allocator.h>

// LLVMFuzzerTestOneInput function signature
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early return if input is too small for meaningful fuzzing
    if (size < 2) {
        return 0;
    }

    // Initialize variables
    rosidl_runtime_c__String__Sequence seq1 = {0};
    rosidl_runtime_c__String__Sequence seq2 = {0};
    rosidl_runtime_c__boolean__Sequence bool_seq1 = {0};
    rosidl_runtime_c__boolean__Sequence bool_seq2 = {0};
    
    // Use fuzz data to determine sequence sizes
    // Use first byte for seq1 size (modulo reasonable limit to avoid excessive allocation)
    size_t seq1_size = (data[0] % 16) + 1;  // 1-16 elements
    // Use second byte for seq2 size
    size_t seq2_size = (data[1] % 16) + 1;  // 1-16 elements
    
    // Initialize first string sequence
    if (!rosidl_runtime_c__String__Sequence__init(&seq1, seq1_size)) {
        // Initialization failed, clean up and return
        goto cleanup;
    }
    
    // Initialize second string sequence
    if (!rosidl_runtime_c__String__Sequence__init(&seq2, seq2_size)) {
        goto cleanup;
    }
    
    // Populate string sequences with data from fuzz input
    size_t data_offset = 2;
    for (size_t i = 0; i < seq1_size && data_offset < size; i++) {
        // Calculate string length from fuzz data (1-32 chars)
        size_t str_len = (data[data_offset % size] % 32) + 1;
        data_offset = (data_offset + 1) % size;
        
        // Ensure we don't read beyond buffer
        if (data_offset + str_len > size) {
            str_len = size - data_offset;
            if (str_len == 0) break;
        }
        
        // Resize string if needed
        if (!rosidl_runtime_c__String__resize(&seq1.data[i], str_len)) {
            goto cleanup;
        }
        
        // Copy fuzz data into string
        if (str_len > 0) {
            memcpy(seq1.data[i].data, data + data_offset, str_len);
            seq1.data[i].data[str_len] = '\0';  // Ensure null termination
        }
        
        data_offset = (data_offset + str_len) % size;
    }
    
    // Populate second sequence similarly
    for (size_t i = 0; i < seq2_size && data_offset < size; i++) {
        size_t str_len = (data[data_offset % size] % 32) + 1;
        data_offset = (data_offset + 1) % size;
        
        if (data_offset + str_len > size) {
            str_len = size - data_offset;
            if (str_len == 0) break;
        }
        
        if (!rosidl_runtime_c__String__resize(&seq2.data[i], str_len)) {
            goto cleanup;
        }
        
        if (str_len > 0) {
            memcpy(seq2.data[i].data, data + data_offset, str_len);
            seq2.data[i].data[str_len] = '\0';
        }
        
        data_offset = (data_offset + str_len) % size;
    }
    
    // Test rosidl_runtime_c__String__Sequence__are_equal
    bool strings_equal = rosidl_runtime_c__String__Sequence__are_equal(&seq1, &seq2);
    (void)strings_equal;  // Use result to avoid unused variable warning
    
    // Initialize boolean sequences for testing
    // Use fuzz data to determine boolean sequence sizes
    size_t bool_seq1_size = (data[0] % 8) + 1;  // 1-8 elements
    size_t bool_seq2_size = (data[1] % 8) + 1;  // 1-8 elements
    
    if (!rosidl_runtime_c__boolean__Sequence__init(&bool_seq1, bool_seq1_size)) {
        goto cleanup;
    }
    
    if (!rosidl_runtime_c__boolean__Sequence__init(&bool_seq2, bool_seq2_size)) {
        rosidl_runtime_c__boolean__Sequence__fini(&bool_seq1);
        goto cleanup;
    }
    
    // Populate boolean sequences with fuzz data
    data_offset = 2;
    for (size_t i = 0; i < bool_seq1_size && data_offset < size; i++) {
        bool_seq1.data[i] = (data[data_offset] & 0x01) != 0;
        data_offset = (data_offset + 1) % size;
    }
    
    for (size_t i = 0; i < bool_seq2_size && data_offset < size; i++) {
        bool_seq2.data[i] = (data[data_offset] & 0x01) != 0;
        data_offset = (data_offset + 1) % size;
    }
    
    // Test rosidl_runtime_c__bool__Sequence__are_equal
    bool bools_equal = rosidl_runtime_c__bool__Sequence__are_equal(&bool_seq1, &bool_seq2);
    (void)bools_equal;
    
    // Test get_message_typesupport_handle_function with dummy data
    // Create minimal dummy structures for testing
    static const char test_identifier[] = "test_identifier";
    static const char other_identifier[] = "other_identifier";
    
    // Create a minimal rosidl_message_type_support_t structure
    rosidl_message_type_support_t dummy_msg_ts = {
        .typesupport_identifier = test_identifier,
        .data = NULL,
        .func = NULL
    };
    
    // Test with matching identifier
    const rosidl_message_type_support_t *result1 = 
        get_message_typesupport_handle_function(&dummy_msg_ts, test_identifier);
    (void)result1;
    
    // Test with non-matching identifier
    const rosidl_message_type_support_t *result2 = 
        get_message_typesupport_handle_function(&dummy_msg_ts, other_identifier);
    (void)result2;
    
    // Test get_service_typesupport_handle with dummy data
    // Create a minimal rosidl_service_type_support_t structure
    rosidl_service_type_support_t dummy_svc_ts = {
        .typesupport_identifier = test_identifier,
        .data = NULL,
        .func = NULL  // Note: This would normally be a function pointer
    };
    
    // Note: We can't actually call get_service_typesupport_handle with this
    // dummy structure because func is NULL, which would cause a crash.
    // We'll just declare it to show the API usage pattern.
    // const rosidl_service_type_support_t *svc_result = 
    //     get_service_typesupport_handle(&dummy_svc_ts, test_identifier);

cleanup:
    // Clean up all allocated resources
    if (seq1.data) {
        rosidl_runtime_c__String__Sequence__fini(&seq1);
    }
    if (seq2.data) {
        rosidl_runtime_c__String__Sequence__fini(&seq2);
    }
    if (bool_seq1.data) {
        rosidl_runtime_c__boolean__Sequence__fini(&bool_seq1);
    }
    if (bool_seq2.data) {
        rosidl_runtime_c__boolean__Sequence__fini(&bool_seq2);
    }
    
    return 0;
}

// Main function for standalone testing (not used by libFuzzer)
#ifdef STANDALONE_TEST
#include <stdio.h>
int main() {
    // Test with some sample data
    uint8_t test_data[] = {5, 5, 'H', 'e', 'l', 'l', 'o', 'W', 'o', 'r', 'l', 'd'};
    LLVMFuzzerTestOneInput(test_data, sizeof(test_data));
    printf("Fuzz test completed successfully\n");
    return 0;
}
#endif
