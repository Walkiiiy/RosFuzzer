#include <rosidl_runtime_c/string.h>
#include <rosidl_runtime_c/string_functions.h>
#include <rosidl_runtime_c/primitives_sequence_functions.h>
#include <rosidl_runtime_c/sequence_bound.h>
#include <rosidl_runtime_c/u16string_functions.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <rcutils/allocator.h>

// Fuzz driver entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early exit for insufficient input
    if (size < 2) {
        return 0;
    }

    // Initialize default allocator
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    
    // Create and initialize first string
    rosidl_runtime_c__String str1;
    if (!rosidl_runtime_c__String__init(&str1)) {
        return 0; // Initialization failed
    }

    // Use fuzz data to create a string content (safe copy)
    // Ensure we don't overflow by limiting to available size
    size_t str_size = size > 0 ? size : 1;
    char *temp_data = (char *)allocator.allocate(str_size + 1, allocator.state);
    if (!temp_data) {
        rosidl_runtime_c__String__fini(&str1);
        return 0;
    }
    
    // Copy fuzz data safely
    size_t copy_size = size < str_size ? size : str_size;
    memcpy(temp_data, data, copy_size);
    temp_data[copy_size] = '\0';
    
    // Assign to string using assignn (implied by copy function)
    // First need to free initial allocation
    allocator.deallocate(str1.data, allocator.state);
    str1.data = temp_data;
    str1.size = copy_size;
    str1.capacity = str_size + 1;

    // Create and initialize second string
    rosidl_runtime_c__String str2;
    if (!rosidl_runtime_c__String__init(&str2)) {
        rosidl_runtime_c__String__fini(&str1);
        return 0;
    }

    // Test rosidl_runtime_c__String__copy
    bool copy_result = rosidl_runtime_c__String__copy(&str1, &str2);
    if (!copy_result) {
        rosidl_runtime_c__String__fini(&str1);
        rosidl_runtime_c__String__fini(&str2);
        return 0;
    }

    // Test rosidl_runtime_c__String__are_equal
    bool equal_result = rosidl_runtime_c__String__are_equal(&str1, &str2);
    // Should be true after copy
    (void)equal_result; // Mark as used

    // Create and initialize string sequences
    rosidl_runtime_c__String__Sequence seq1;
    rosidl_runtime_c__String__Sequence seq2;
    
    // Initialize sequences with capacity 2
    seq1.capacity = 2;
    seq1.size = 2;
    seq1.data = (rosidl_runtime_c__String *)allocator.allocate(
        2 * sizeof(rosidl_runtime_c__String), allocator.state);
    
    if (!seq1.data) {
        rosidl_runtime_c__String__fini(&str1);
        rosidl_runtime_c__String__fini(&str2);
        return 0;
    }
    
    // Initialize all strings in sequence 1
    for (size_t i = 0; i < 2; ++i) {
        if (!rosidl_runtime_c__String__init(&seq1.data[i])) {
            // Clean up already initialized strings
            for (size_t j = 0; j < i; ++j) {
                rosidl_runtime_c__String__fini(&seq1.data[j]);
            }
            allocator.deallocate(seq1.data, allocator.state);
            rosidl_runtime_c__String__fini(&str1);
            rosidl_runtime_c__String__fini(&str2);
            return 0;
        }
        
        // Assign some data to sequence strings (using first half of input for first,
        // second half for second, if available)
        size_t offset = (i * size) / 2;
        size_t segment_size = size / 2;
        if (segment_size > 0 && offset < size) {
            // Create temporary buffer
            char *seg_data = (char *)allocator.allocate(segment_size + 1, allocator.state);
            if (seg_data) {
                size_t actual_copy = (offset + segment_size <= size) ? 
                    segment_size : size - offset;
                memcpy(seg_data, data + offset, actual_copy);
                seg_data[actual_copy] = '\0';
                
                // Replace string data
                allocator.deallocate(seq1.data[i].data, allocator.state);
                seq1.data[i].data = seg_data;
                seq1.data[i].size = actual_copy;
                seq1.data[i].capacity = segment_size + 1;
            }
        }
    }
    
    // Initialize sequence 2 with zero capacity
    seq2.capacity = 0;
    seq2.size = 0;
    seq2.data = NULL;
    
    // Test rosidl_runtime_c__String__Sequence__copy
    bool seq_copy_result = rosidl_runtime_c__String__Sequence__copy(&seq1, &seq2);
    (void)seq_copy_result; // Mark as used
    
    // Clean up sequence 1
    for (size_t i = 0; i < seq1.size; ++i) {
        rosidl_runtime_c__String__fini(&seq1.data[i]);
    }
    allocator.deallocate(seq1.data, allocator.state);
    
    // Clean up sequence 2 if it was successfully copied
    if (seq2.data) {
        for (size_t i = 0; i < seq2.size; ++i) {
            rosidl_runtime_c__String__fini(&seq2.data[i]);
        }
        allocator.deallocate(seq2.data, allocator.state);
    }
    
    // Clean up individual strings
    rosidl_runtime_c__String__fini(&str1);
    rosidl_runtime_c__String__fini(&str2);
    
    return 0;
}
