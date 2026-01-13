#include <rosidl_runtime_c/string.h>
#include <rosidl_runtime_c/primitives_sequence.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <rcutils/allocator.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early exit if input is too small for meaningful testing
    if (size < 2) {
        return 0;
    }

    // Initialize variables
    rosidl_runtime_c__String str1 = {0};
    rosidl_runtime_c__String str2 = {0};
    rosidl_runtime_c__String__Sequence seq1 = {0};
    rosidl_runtime_c__String__Sequence seq2 = {0};
    
    bool init_success = false;
    bool seq_init_success1 = false;
    bool seq_init_success2 = false;
    
    // Use first byte to determine sequence sizes (with bounds)
    size_t seq_size1 = (data[0] % 8) + 1;  // Range: 1-8
    size_t seq_size2 = (data[1] % 8) + 1;  // Range: 1-8
    
    // Initialize individual strings
    if (!rosidl_runtime_c__String__init(&str1)) {
        goto cleanup;
    }
    
    if (!rosidl_runtime_c__String__init(&str2)) {
        rosidl_runtime_c__String__fini(&str1);
        goto cleanup;
    }
    
    init_success = true;
    
    // Initialize string sequences
    if (!rosidl_runtime_c__String__Sequence__init(&seq1, seq_size1)) {
        goto cleanup;
    }
    seq_init_success1 = true;
    
    if (!rosidl_runtime_c__String__Sequence__init(&seq2, seq_size2)) {
        goto cleanup;
    }
    seq_init_success2 = true;
    
    // Test string equality with empty strings
    bool equal_empty = rosidl_runtime_c__String__are_equal(&str1, &str2);
    (void)equal_empty;  // Use result to avoid unused variable warning
    
    // Test sequence equality (different sizes should return false)
    bool seq_equal_diff_sizes = rosidl_runtime_c__String__Sequence__are_equal(&seq1, &seq2);
    (void)seq_equal_diff_sizes;
    
    // Test sequence equality with same-sized sequences
    // Create a third sequence with same size as seq1
    rosidl_runtime_c__String__Sequence seq3 = {0};
    bool seq_init_success3 = false;
    
    if (rosidl_runtime_c__String__Sequence__init(&seq3, seq_size1)) {
        seq_init_success3 = true;
        
        // Compare seq1 with seq3 (both should be empty strings)
        bool seq_equal_same_size = rosidl_runtime_c__String__Sequence__are_equal(&seq1, &seq3);
        (void)seq_equal_same_size;
        
        // Clean up seq3
        if (seq_init_success3) {
            // Free individual strings in sequence
            for (size_t i = 0; i < seq3.size; ++i) {
                rosidl_runtime_c__String__fini(&seq3.data[i]);
            }
            // Free sequence data
            rcutils_allocator_t allocator = rcutils_get_default_allocator();
            allocator.deallocate(seq3.data, allocator.state);
        }
    }
    
    // Test with NULL pointers for safety
    bool null_test1 = rosidl_runtime_c__String__are_equal(NULL, &str1);
    bool null_test2 = rosidl_runtime_c__String__are_equal(&str1, NULL);
    bool null_test3 = rosidl_runtime_c__String__are_equal(NULL, NULL);
    (void)null_test1;
    (void)null_test2;
    (void)null_test3;
    
    bool seq_null_test1 = rosidl_runtime_c__String__Sequence__are_equal(NULL, &seq1);
    bool seq_null_test2 = rosidl_runtime_c__String__Sequence__are_equal(&seq1, NULL);
    bool seq_null_test3 = rosidl_runtime_c__String__Sequence__are_equal(NULL, NULL);
    (void)seq_null_test1;
    (void)seq_null_test2;
    (void)seq_null_test3;
    
    // Test with some actual string data if we have enough input
    if (size > 10) {
        // Use a small portion of fuzz data for string content
        size_t str_data_len = (size - 2) % 256;
        if (str_data_len > 0) {
            // Note: We're not modifying the strings since they're initialized empty
            // In a real scenario, we might allocate and copy data here
            // But per API requirements, we're only testing the provided functions
        }
    }

cleanup:
    // Clean up sequences (in reverse order of creation)
    if (seq_init_success2) {
        // Free individual strings in sequence
        for (size_t i = 0; i < seq2.size; ++i) {
            rosidl_runtime_c__String__fini(&seq2.data[i]);
        }
        // Free sequence data
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        allocator.deallocate(seq2.data, allocator.state);
    }
    
    if (seq_init_success1) {
        // Free individual strings in sequence
        for (size_t i = 0; i < seq1.size; ++i) {
            rosidl_runtime_c__String__fini(&seq1.data[i]);
        }
        // Free sequence data
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        allocator.deallocate(seq1.data, allocator.state);
    }
    
    // Clean up individual strings
    if (init_success) {
        rosidl_runtime_c__String__fini(&str2);
        rosidl_runtime_c__String__fini(&str1);
    }
    
    return 0;
}
