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

// LLVMFuzzerTestOneInput function signature
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

/**
 * @brief Fuzz driver for rosidl_runtime_c string sequence functions
 * 
 * This fuzz driver tests the following APIs:
 * 1. rosidl_runtime_c__String__Sequence__init
 * 2. rosidl_runtime_c__String__Sequence__copy
 * 3. rosidl_runtime_c__String__Sequence__are_equal
 * 4. rosidl_runtime_c__String__Sequence__fini
 * 5. rosidl_runtime_c__String__Sequence__destroy
 * 
 * The fuzzer input is used to:
 * - Determine sequence sizes
 * - Provide string content
 * - Control test flow
 */
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early return if insufficient data for basic operations
    if (size < 2) {
        return 0;
    }

    // Use first byte to determine sequence size (modulo to keep it reasonable)
    size_t seq_size = (data[0] % 16) + 1;  // Range: 1-16
    
    // Calculate available data for strings (skip the size byte)
    size_t string_data_size = size - 1;
    const uint8_t *string_data = data + 1;

    // Initialize first sequence
    rosidl_runtime_c__String__Sequence seq1;
    if (!rosidl_runtime_c__String__Sequence__init(&seq1, seq_size)) {
        // If initialization fails, clean up and return
        return 0;
    }

    // Fill sequence with data from fuzz input
    for (size_t i = 0; i < seq1.size && i * 16 < string_data_size; i++) {
        // Calculate string length for this element (max 15 chars + null terminator)
        size_t str_len = (string_data[i * 16] % 15) + 1;
        
        // Calculate available data for this string
        size_t data_offset = i * 16 + 1;
        if (data_offset >= string_data_size) {
            break;
        }
        
        size_t available_len = string_data_size - data_offset;
        if (available_len > str_len) {
            available_len = str_len;
        }
        
        // Ensure we don't overflow
        if (available_len > 0) {
            // Allocate buffer for string copy
            char *temp_buf = (char *)malloc(available_len + 1);
            if (!temp_buf) {
                rosidl_runtime_c__String__Sequence__fini(&seq1);
                return 0;
            }
            
            // Copy data and null-terminate
            memcpy(temp_buf, string_data + data_offset, available_len);
            temp_buf[available_len] = '\0';
            
            // Assign to sequence string
            seq1.data[i].data = temp_buf;
            seq1.data[i].size = available_len;
            seq1.data[i].capacity = available_len + 1;
            
            // Note: We're directly manipulating the string structure
            // because rosidl_runtime_c__String__assignn might not be available
            // This is acceptable for fuzzing purposes
        }
    }

    // Initialize second sequence with different size
    size_t seq2_size = (data[0] % 8) + 1;  // Range: 1-8
    rosidl_runtime_c__String__Sequence seq2;
    if (!rosidl_runtime_c__String__Sequence__init(&seq2, seq2_size)) {
        rosidl_runtime_c__String__Sequence__fini(&seq1);
        return 0;
    }

    // Test copy function
    bool copy_result = rosidl_runtime_c__String__Sequence__copy(&seq1, &seq2);
    
    // Test are_equal function (should be false if sizes differ)
    bool equal_result = rosidl_runtime_c__String__Sequence__are_equal(&seq1, &seq2);
    
    // If copy succeeded and sizes match, sequences should be equal
    if (copy_result && seq1.size == seq2.size) {
        // Verify equality after successful copy
        bool verify_equal = rosidl_runtime_c__String__Sequence__are_equal(&seq1, &seq2);
        (void)verify_equal;  // Mark as used to avoid compiler warning
    }

    // Create a third sequence for copy testing
    rosidl_runtime_c__String__Sequence seq3;
    if (!rosidl_runtime_c__String__Sequence__init(&seq3, 0)) {
        rosidl_runtime_c__String__Sequence__fini(&seq2);
        rosidl_runtime_c__String__Sequence__fini(&seq1);
        return 0;
    }

    // Test copy to empty sequence
    bool copy_to_empty = rosidl_runtime_c__String__Sequence__copy(&seq1, &seq3);
    (void)copy_to_empty;  // Mark as used

    // Test equality with copied sequence
    if (copy_to_empty) {
        bool equal_after_copy = rosidl_runtime_c__String__Sequence__are_equal(&seq1, &seq3);
        (void)equal_after_copy;  // Mark as used
    }

    // Test with NULL pointers (error cases)
    bool null_test1 = rosidl_runtime_c__String__Sequence__are_equal(NULL, &seq1);
    bool null_test2 = rosidl_runtime_c__String__Sequence__are_equal(&seq1, NULL);
    bool null_test3 = rosidl_runtime_c__String__Sequence__copy(NULL, &seq1);
    bool null_test4 = rosidl_runtime_c__String__Sequence__copy(&seq1, NULL);
    (void)null_test1; (void)null_test2; (void)null_test3; (void)null_test4;

    // Test fini with NULL
    rosidl_runtime_c__String__Sequence__fini(NULL);

    // Dynamically allocate a sequence for destroy test
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    rosidl_runtime_c__String__Sequence *dyn_seq = 
        (rosidl_runtime_c__String__Sequence *)allocator.allocate(
            sizeof(rosidl_runtime_c__String__Sequence), allocator.state);
    
    if (dyn_seq) {
        // Initialize the dynamically allocated sequence
        if (rosidl_runtime_c__String__Sequence__init(dyn_seq, 2)) {
            // Test destroy function
            rosidl_runtime_c__String__Sequence__destroy(dyn_seq);
            dyn_seq = NULL;  // Pointer is now invalid
        } else {
            // If init failed, free the memory
            allocator.deallocate(dyn_seq, allocator.state);
        }
    }

    // Test destroy with NULL
    rosidl_runtime_c__String__Sequence__destroy(NULL);

    // Clean up all sequences
    rosidl_runtime_c__String__Sequence__fini(&seq3);
    rosidl_runtime_c__String__Sequence__fini(&seq2);
    rosidl_runtime_c__String__Sequence__fini(&seq1);

    // Free any allocated string buffers
    for (size_t i = 0; i < seq1.size && i * 16 < string_data_size; i++) {
        if (seq1.data[i].data) {
            free((void *)seq1.data[i].data);
        }
    }

    return 0;
}

// Main function for standalone testing (not used during fuzzing)
#ifdef STANDALONE_TEST
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }
    
    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        printf("Failed to open file\n");
        return 1;
    }
    
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    uint8_t *data = (uint8_t *)malloc(file_size);
    if (!data) {
        fclose(f);
        printf("Failed to allocate memory\n");
        return 1;
    }
    
    size_t bytes_read = fread(data, 1, file_size, f);
    fclose(f);
    
    if (bytes_read != file_size) {
        free(data);
        printf("Failed to read file\n");
        return 1;
    }
    
    int result = LLVMFuzzerTestOneInput(data, bytes_read);
    free(data);
    
    return result;
}
#endif
