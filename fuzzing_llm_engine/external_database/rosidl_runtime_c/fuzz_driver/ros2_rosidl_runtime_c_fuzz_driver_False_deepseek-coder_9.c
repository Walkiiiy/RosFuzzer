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
 * 3. rosidl_runtime_c__String__Sequence__fini
 * 4. rosidl_runtime_c__String__Sequence__create
 * 5. rosidl_runtime_c__String__Sequence__are_equal
 * 
 * The fuzz input is used to determine sequence sizes and populate string data.
 */
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early return if insufficient data
    if (size < 2) {
        return 0;
    }

    // Use first byte to determine sequence size (modulo to keep it reasonable)
    size_t seq_size = (data[0] % 16) + 1;  // Range: 1-16
    
    // Ensure we have enough data for the strings
    if (size < 1 + seq_size) {
        return 0;
    }

    // Calculate available data for string content
    size_t string_data_size = size - 1;
    const uint8_t *string_data = data + 1;

    // Test 1: Create and initialize sequence using init()
    rosidl_runtime_c__String__Sequence seq1;
    memset(&seq1, 0, sizeof(seq1));
    
    bool success = rosidl_runtime_c__String__Sequence__init(&seq1, seq_size);
    if (!success) {
        // If init fails, ensure seq1 is in a clean state
        assert(seq1.data == NULL);
        assert(seq1.size == 0);
        assert(seq1.capacity == 0);
        return 0;
    }

    // Populate the strings with fuzz data
    for (size_t i = 0; i < seq_size && i < seq1.capacity; i++) {
        // Calculate string length for this element
        // Use modulo to ensure we don't exceed available data
        size_t str_len = (string_data[i % string_data_size] % 32) + 1;
        
        // Ensure we don't exceed available data
        size_t data_offset = (i * 7) % string_data_size;  // Simple hash for variety
        size_t available = string_data_size - data_offset;
        str_len = str_len < available ? str_len : available;
        
        if (str_len > 0) {
            // Allocate and copy string data
            char *str_buf = (char *)malloc(str_len + 1);
            if (str_buf) {
                memcpy(str_buf, string_data + data_offset, str_len);
                str_buf[str_len] = '\0';
                
                // Assign to string element
                seq1.data[i].data = str_buf;
                seq1.data[i].size = str_len;
                seq1.data[i].capacity = str_len + 1;
            }
        }
    }

    // Test 2: Create another sequence using create()
    rosidl_runtime_c__String__Sequence *seq2 = 
        rosidl_runtime_c__String__Sequence__create(seq_size);
    if (!seq2) {
        // Clean up seq1 before returning
        rosidl_runtime_c__String__Sequence__fini(&seq1);
        return 0;
    }

    // Populate seq2 with different data
    for (size_t i = 0; i < seq_size && i < seq2->capacity; i++) {
        // Calculate string length for this element
        size_t str_len = (string_data[(i + 5) % string_data_size] % 32) + 1;
        
        // Ensure we don't exceed available data
        size_t data_offset = ((i + 3) * 11) % string_data_size;
        size_t available = string_data_size - data_offset;
        str_len = str_len < available ? str_len : available;
        
        if (str_len > 0) {
            // Allocate and copy string data
            char *str_buf = (char *)malloc(str_len + 1);
            if (str_buf) {
                memcpy(str_buf, string_data + data_offset, str_len);
                str_buf[str_len] = '\0';
                
                // Assign to string element
                seq2->data[i].data = str_buf;
                seq2->data[i].size = str_len;
                seq2->data[i].capacity = str_len + 1;
            }
        }
    }

    // Test 3: Copy seq1 to seq2 using copy()
    // First, clean up seq2's current content
    rosidl_runtime_c__String__Sequence__fini(seq2);
    
    // Reinitialize seq2 with different size to test reallocation
    success = rosidl_runtime_c__String__Sequence__init(seq2, seq_size / 2);
    if (!success) {
        // Clean up and return
        rosidl_runtime_c__String__Sequence__fini(&seq1);
        free(seq2);
        return 0;
    }

    // Now copy seq1 to seq2 (should trigger reallocation)
    success = rosidl_runtime_c__String__Sequence__copy(&seq1, seq2);
    if (!success) {
        // Copy failed, clean up and continue
        rosidl_runtime_c__String__Sequence__fini(&seq1);
        rosidl_runtime_c__String__Sequence__fini(seq2);
        free(seq2);
        return 0;
    }

    // Test 4: Create a third sequence and copy seq2 to it
    rosidl_runtime_c__String__Sequence seq3;
    memset(&seq3, 0, sizeof(seq3));
    
    success = rosidl_runtime_c__String__Sequence__init(&seq3, seq_size);
    if (!success) {
        rosidl_runtime_c__String__Sequence__fini(&seq1);
        rosidl_runtime_c__String__Sequence__fini(seq2);
        free(seq2);
        return 0;
    }

    success = rosidl_runtime_c__String__Sequence__copy(seq2, &seq3);
    if (!success) {
        // Clean up and continue
        rosidl_runtime_c__String__Sequence__fini(&seq1);
        rosidl_runtime_c__String__Sequence__fini(seq2);
        rosidl_runtime_c__String__Sequence__fini(&seq3);
        free(seq2);
        return 0;
    }

    // Test 5: Check equality
    // seq1 and seq3 should be equal (seq3 is a copy of seq2 which is a copy of seq1)
    bool are_equal = rosidl_runtime_c__String__Sequence__are_equal(&seq1, &seq3);
    (void)are_equal;  // Use result to avoid unused variable warning
    
    // Also test with NULL pointers
    bool null_test1 = rosidl_runtime_c__String__Sequence__are_equal(NULL, &seq1);
    bool null_test2 = rosidl_runtime_c__String__Sequence__are_equal(&seq1, NULL);
    bool null_test3 = rosidl_runtime_c__String__Sequence__are_equal(NULL, NULL);
    (void)null_test1;
    (void)null_test2;
    (void)null_test3;

    // Test copy with NULL pointers
    bool copy_null1 = rosidl_runtime_c__String__Sequence__copy(NULL, &seq3);
    bool copy_null2 = rosidl_runtime_c__String__Sequence__copy(&seq1, NULL);
    (void)copy_null1;
    (void)copy_null2;

    // Test init with NULL pointer
    bool init_null = rosidl_runtime_c__String__Sequence__init(NULL, 5);
    (void)init_null;

    // Test fini with NULL pointer (should be safe)
    rosidl_runtime_c__String__Sequence__fini(NULL);

    // Clean up all allocated resources
    // First, free the string data we allocated manually
    for (size_t i = 0; i < seq1.capacity; i++) {
        if (seq1.data[i].data) {
            free(seq1.data[i].data);
            seq1.data[i].data = NULL;
        }
    }
    
    for (size_t i = 0; i < seq2->capacity; i++) {
        if (seq2->data[i].data) {
            free(seq2->data[i].data);
            seq2->data[i].data = NULL;
        }
    }
    
    for (size_t i = 0; i < seq3.capacity; i++) {
        if (seq3.data[i].data) {
            free(seq3.data[i].data);
            seq3.data[i].data = NULL;
        }
    }

    // Now finalize the sequences
    rosidl_runtime_c__String__Sequence__fini(&seq1);
    rosidl_runtime_c__String__Sequence__fini(seq2);
    rosidl_runtime_c__String__Sequence__fini(&seq3);
    
    // Free the dynamically allocated sequence
    free(seq2);

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
        printf("Error opening file\n");
        return 1;
    }
    
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    uint8_t *data = (uint8_t *)malloc(fsize);
    if (!data) {
        fclose(f);
        printf("Memory allocation failed\n");
        return 1;
    }
    
    size_t read = fread(data, 1, fsize, f);
    fclose(f);
    
    if (read != fsize) {
        free(data);
        printf("File read error\n");
        return 1;
    }
    
    int result = LLVMFuzzerTestOneInput(data, fsize);
    free(data);
    
    return result;
}
#endif
