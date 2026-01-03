#include <rosidl_runtime_c/message_type_support.h>
#include <rosidl_runtime_c/primitives_sequence_functions.h>
#include <rosidl_runtime_c/sequence_bound.h>
#include <rosidl_runtime_c/service_type_support.h>
#include <rosidl_runtime_c/string_functions.h>
#include <rosidl_runtime_c/u16string_functions.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

// Note: <sstream> is a C++ header, but we're writing C code.
// We'll use C string functions instead.

// Forward declarations for types that might be needed
typedef struct rosidl_runtime_c__String__Sequence rosidl_runtime_c__String__Sequence;
typedef struct rosidl_runtime_c__boolean__Sequence rosidl_runtime_c__boolean__Sequence;
typedef struct rosidl_runtime_c__Sequence__bound rosidl_runtime_c__Sequence__bound;
typedef struct rosidl_service_type_support_t rosidl_service_type_support_t;

// Function pointer type for service typesupport handle function
typedef const rosidl_service_type_support_t *(*rosidl_service_typesupport_handle_function)(
    const rosidl_service_type_support_t *, const char *);

// LLVMFuzzerTestOneInput function
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early return if no data
    if (data == NULL || size == 0) {
        return 0;
    }

    // Initialize variables
    rosidl_runtime_c__String__Sequence str_seq1 = {0};
    rosidl_runtime_c__String__Sequence str_seq2 = {0};
    rosidl_runtime_c__boolean__Sequence bool_seq1 = {0};
    rosidl_runtime_c__boolean__Sequence bool_seq2 = {0};
    rosidl_runtime_c__Sequence__bound seq_bound_handle = {0};
    rosidl_service_type_support_t service_ts_handle = {0};
    
    bool success = false;
    char *identifier = NULL;
    size_t seq_size = 0;
    const rosidl_runtime_c__Sequence__bound *bound_result = NULL;
    const rosidl_service_type_support_t *service_result = NULL;

    // 1. Test rosidl_runtime_c__String__Sequence__init
    // Derive sequence size from fuzz input (limit to reasonable size)
    seq_size = size % 16;  // Limit to max 15 elements to avoid excessive memory usage
    
    if (seq_size > 0) {
        // Initialize first string sequence
        success = rosidl_runtime_c__String__Sequence__init(&str_seq1, seq_size);
        if (!success) {
            // Cleanup and return if initialization fails
            goto cleanup;
        }
        
        // Initialize second string sequence with same size
        success = rosidl_runtime_c__String__Sequence__init(&str_seq2, seq_size);
        if (!success) {
            goto cleanup;
        }
        
        // Test rosidl_runtime_c__String__fini on individual strings
        // Clean up a few strings from the sequences
        for (size_t i = 0; i < seq_size && i < 3; i++) {
            rosidl_runtime_c__String__fini(&str_seq1.data[i]);
            // Re-initialize the string after fini
            // Note: In real usage, we'd need rosidl_runtime_c__String__init
            // but we're just testing the fini function
        }
    }

    // 2. Test rosidl_runtime_c__bool__Sequence__are_equal
    // Initialize boolean sequences for comparison
    // First, allocate memory for boolean sequences
    bool_seq1.size = seq_size;
    bool_seq2.size = seq_size;
    
    if (seq_size > 0) {
        bool_seq1.data = (bool*)calloc(seq_size, sizeof(bool));
        bool_seq2.data = (bool*)calloc(seq_size, sizeof(bool));
        
        if (!bool_seq1.data || !bool_seq2.data) {
            goto cleanup;
        }
        
        // Fill sequences with data from fuzz input
        for (size_t i = 0; i < seq_size; i++) {
            bool_seq1.data[i] = (data[i % size] % 2) == 0;
            bool_seq2.data[i] = (data[i % size] % 2) == 0;  // Same data for equality
        }
        
        // Test equality - should return true since we filled them identically
        bool are_equal = rosidl_runtime_c__bool__Sequence__are_equal(&bool_seq1, &bool_seq2);
        (void)are_equal;  // Mark as used to avoid compiler warning
    }

    // 3. Test get_sequence_bound_handle_function
    // Create identifier from fuzz data
    size_t id_len = (size % 32) + 1;  // Ensure at least 1 byte
    if (id_len > size) {
        id_len = size;
    }
    
    identifier = (char*)malloc(id_len + 1);
    if (!identifier) {
        goto cleanup;
    }
    
    // Copy data to identifier, ensuring null termination
    memcpy(identifier, data, id_len);
    identifier[id_len] = '\0';
    
    // Initialize sequence bound handle
    seq_bound_handle.typesupport_identifier = identifier;
    
    // Test with matching identifier
    bound_result = get_sequence_bound_handle_function(&seq_bound_handle, identifier);
    (void)bound_result;  // Mark as used
    
    // Test with non-matching identifier
    char dummy_id[] = "non_matching_identifier";
    bound_result = get_sequence_bound_handle_function(&seq_bound_handle, dummy_id);
    (void)bound_result;

    // 4. Test get_service_typesupport_handle
    // Initialize service typesupport handle with a dummy function
    // Note: We need to create a minimal valid function pointer
    service_ts_handle.func = (const void*)(uintptr_t)get_service_typesupport_handle;
    
    // Test the function
    service_result = get_service_typesupport_handle(&service_ts_handle, identifier);
    (void)service_result;

cleanup:
    // Free all allocated resources
    
    // Free string sequences
    if (str_seq1.data) {
        // Finalize all initialized strings in sequence 1
        for (size_t i = 0; i < str_seq1.size; i++) {
            // Only finalize if string was initialized (not already finalized)
            if (i >= 3) {  // We only finalized first 3 strings
                rosidl_runtime_c__String__fini(&str_seq1.data[i]);
            }
        }
        // Free the sequence data array
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        allocator.deallocate(str_seq1.data, allocator.state);
    }
    
    if (str_seq2.data) {
        // Finalize all strings in sequence 2
        for (size_t i = 0; i < str_seq2.size; i++) {
            rosidl_runtime_c__String__fini(&str_seq2.data[i]);
        }
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        allocator.deallocate(str_seq2.data, allocator.state);
    }
    
    // Free boolean sequences
    if (bool_seq1.data) {
        free(bool_seq1.data);
    }
    if (bool_seq2.data) {
        free(bool_seq2.data);
    }
    
    // Free identifier
    if (identifier) {
        free(identifier);
    }
    
    // Note: We don't need to free seq_bound_handle or service_ts_handle
    // as they were stack-allocated and don't own dynamic memory
    
    return 0;
}

// Main function for standalone testing (not used by libFuzzer)
#ifdef STANDALONE_TEST
#include <stdio.h>
int main() {
    // Test with sample data
    uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    size_t test_size = sizeof(test_data);
    
    printf("Running fuzz driver test...\n");
    int result = LLVMFuzzerTestOneInput(test_data, test_size);
    printf("Test completed with result: %d\n", result);
    
    return 0;
}
#endif
