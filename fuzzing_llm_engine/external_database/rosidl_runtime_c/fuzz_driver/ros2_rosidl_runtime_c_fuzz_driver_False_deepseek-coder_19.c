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

// LLVMFuzzerTestOneInput - Main fuzzing entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early return if input is too small for meaningful fuzzing
    if (size < 2) {
        return 0;
    }

    // Initialize sequences
    rosidl_runtime_c__U16String__Sequence seq1 = {0};
    rosidl_runtime_c__U16String__Sequence seq2 = {0};
    rosidl_runtime_c__U16String__Sequence seq3 = {0};

    // Determine sequence size from fuzz input (1-16 elements)
    size_t seq_size = (data[0] % 16) + 1;
    if (seq_size > size / 2) {
        seq_size = size / 2;
        if (seq_size == 0) {
            return 0;
        }
    }

    // Initialize first sequence
    if (!rosidl_runtime_c__U16String__Sequence__init(&seq1, seq_size)) {
        goto cleanup;
    }

    // Populate seq1 with data from fuzz input
    for (size_t i = 0; i < seq_size; ++i) {
        // Calculate string length (1-64 characters)
        size_t str_len = (data[(i % size) + 1] % 64) + 1;
        if (str_len * sizeof(uint16_t) > size - (i * sizeof(uint16_t))) {
            str_len = (size - (i * sizeof(uint16_t))) / sizeof(uint16_t);
            if (str_len == 0) {
                str_len = 1;
            }
        }

        // Resize string to hold the data
        if (!rosidl_runtime_c__U16String__resize(&seq1.data[i], str_len)) {
            goto cleanup;
        }

        // Copy data from fuzz input to UTF-16 string
        // Use safe copy with bounds checking
        size_t copy_len = str_len;
        if ((i * sizeof(uint16_t) + copy_len * sizeof(uint16_t)) > size) {
            copy_len = (size - i * sizeof(uint16_t)) / sizeof(uint16_t);
        }
        
        if (copy_len > 0) {
            memcpy(seq1.data[i].data, data + (i * sizeof(uint16_t)), 
                   copy_len * sizeof(uint16_t));
        }
    }

    // Initialize second sequence with same size
    if (!rosidl_runtime_c__U16String__Sequence__init(&seq2, seq_size)) {
        goto cleanup;
    }

    // Copy seq1 to seq2 using the target API
    if (!rosidl_runtime_c__U16String__Sequence__copy(&seq1, &seq2)) {
        goto cleanup;
    }

    // Test sequence equality - should be true after copy
    bool are_equal = rosidl_runtime_c__U16String__Sequence__are_equal(&seq1, &seq2);
    (void)are_equal; // Use result to avoid unused variable warning

    // Initialize third sequence with different size
    size_t seq3_size = seq_size > 1 ? seq_size - 1 : 1;
    if (!rosidl_runtime_c__U16String__Sequence__init(&seq3, seq3_size)) {
        goto cleanup;
    }

    // Test individual string equality for first element
    if (seq_size > 0 && seq3_size > 0) {
        bool str_equal = rosidl_runtime_c__U16String__are_equal(&seq1.data[0], &seq2.data[0]);
        (void)str_equal; // Use result to avoid unused variable warning
    }

    // Test sequence equality with different sized sequences - should be false
    bool diff_equal = rosidl_runtime_c__U16String__Sequence__are_equal(&seq1, &seq3);
    (void)diff_equal; // Use result to avoid unused variable warning

cleanup:
    // Clean up all sequences
    rosidl_runtime_c__U16String__Sequence__fini(&seq1);
    rosidl_runtime_c__U16String__Sequence__fini(&seq2);
    rosidl_runtime_c__U16String__Sequence__fini(&seq3);

    return 0;
}
