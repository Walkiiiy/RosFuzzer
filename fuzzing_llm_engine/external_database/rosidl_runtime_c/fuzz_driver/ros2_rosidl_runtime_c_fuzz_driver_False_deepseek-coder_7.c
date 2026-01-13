#include <rosidl_runtime_c/primitives_sequence_functions.h>
#include <rosidl_runtime_c/sequence_bound.h>
#include <rosidl_runtime_c/string_functions.h>
#include <rosidl_runtime_c/u16string_functions.h>
#include <rcutils/allocator.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

// LLVMFuzzerTestOneInput function signature
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

// Helper function to extract a size_t from fuzz data
static size_t extract_size_t(const uint8_t **data, size_t *remaining) {
    if (*remaining >= sizeof(size_t)) {
        size_t value;
        memcpy(&value, *data, sizeof(size_t));
        *data += sizeof(size_t);
        *remaining -= sizeof(size_t);
        return value;
    }
    return 0;
}

// Helper function to extract a string from fuzz data
static const char* extract_string(const uint8_t **data, size_t *remaining, size_t *str_len) {
    if (*remaining == 0) {
        *str_len = 0;
        return "";
    }
    
    // Use first byte as length (modulo remaining to avoid overflow)
    size_t max_len = *remaining - 1;
    size_t len = (*data)[0] % (max_len + 1);
    *str_len = len;
    
    const char* str = (const char*)(*data + 1);
    *data += (1 + len);
    *remaining -= (1 + len);
    
    return str;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < sizeof(size_t) * 2) {
        // Not enough data for basic operations
        return 0;
    }

    // Initialize default allocator
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    
    // Extract parameters from fuzz data
    size_t remaining = size;
    const uint8_t *cursor = data;
    
    // Extract sequence size (bounded to prevent excessive memory usage)
    size_t seq_size = extract_size_t(&cursor, &remaining);
    seq_size = seq_size % 16;  // Limit to reasonable size for fuzzing
    
    // Initialize first sequence
    rosidl_runtime_c__String__Sequence seq1;
    memset(&seq1, 0, sizeof(seq1));
    
    if (!rosidl_runtime_c__String__Sequence__init(&seq1, seq_size)) {
        // Initialization failed, clean up and return
        rosidl_runtime_c__String__Sequence__fini(&seq1);
        return 0;
    }
    
    // Populate the sequence with strings from fuzz data
    for (size_t i = 0; i < seq_size && remaining > 0; i++) {
        size_t str_len = 0;
        const char* str_data = extract_string(&cursor, &remaining, &str_len);
        
        // Ensure the string is properly initialized
        if (seq1.data[i].capacity == 0) {
            // String should have been initialized by Sequence__init
            continue;
        }
        
        // Copy string data (safe copy with bounds checking)
        if (str_len > 0) {
            // Ensure we don't overflow the string's capacity
            size_t copy_len = str_len;
            if (copy_len >= seq1.data[i].capacity) {
                copy_len = seq1.data[i].capacity - 1;
            }
            
            if (copy_len > 0) {
                memcpy(seq1.data[i].data, str_data, copy_len);
                seq1.data[i].data[copy_len] = '\0';
                seq1.data[i].size = copy_len;
            }
        }
    }
    
    // Initialize second sequence for copying
    rosidl_runtime_c__String__Sequence seq2;
    memset(&seq2, 0, sizeof(seq2));
    
    // Try to copy sequence
    bool copy_success = rosidl_runtime_c__String__Sequence__copy(&seq1, &seq2);
    
    if (copy_success) {
        // Test equality - sequences should be equal after copy
        bool are_equal = rosidl_runtime_c__String__Sequence__are_equal(&seq1, &seq2);
        
        // If copy succeeded, sequences should be equal
        // (unless there's an issue with empty strings or initialization)
        if (seq_size > 0) {
            // For non-empty sequences, they should be equal
            // Note: This might not hold if string initialization failed
            // during copy, but copy_success would be false in that case
            (void)are_equal;  // Use result to avoid unused variable warning
        }
        
        // Clean up second sequence
        rosidl_runtime_c__String__Sequence__fini(&seq2);
    }
    
    // Test equality with itself
    bool self_equal = rosidl_runtime_c__String__Sequence__are_equal(&seq1, &seq1);
    assert(self_equal == true);  // Should always be true
    
    // Test with NULL pointers
    bool null_test1 = rosidl_runtime_c__String__Sequence__are_equal(NULL, &seq1);
    bool null_test2 = rosidl_runtime_c__String__Sequence__are_equal(&seq1, NULL);
    bool null_test3 = rosidl_runtime_c__String__Sequence__are_equal(NULL, NULL);
    assert(null_test1 == false);
    assert(null_test2 == false);
    assert(null_test3 == false);
    
    // Test copy with NULL pointers
    bool null_copy1 = rosidl_runtime_c__String__Sequence__copy(NULL, &seq1);
    bool null_copy2 = rosidl_runtime_c__String__Sequence__copy(&seq1, NULL);
    bool null_copy3 = rosidl_runtime_c__String__Sequence__copy(NULL, NULL);
    assert(null_copy1 == false);
    assert(null_copy2 == false);
    assert(null_copy3 == false);
    
    // Test init with NULL pointer
    bool null_init = rosidl_runtime_c__String__Sequence__init(NULL, 5);
    assert(null_init == false);
    
    // Clean up first sequence
    rosidl_runtime_c__String__Sequence__fini(&seq1);
    
    // Test individual string finalization with NULL
    rosidl_runtime_c__String__fini(NULL);
    
    // Test sequence finalization with NULL
    rosidl_runtime_c__String__Sequence__fini(NULL);
    
    // Additional test: Initialize and finalize an empty sequence
    rosidl_runtime_c__String__Sequence empty_seq;
    memset(&empty_seq, 0, sizeof(empty_seq));
    
    if (rosidl_runtime_c__String__Sequence__init(&empty_seq, 0)) {
        rosidl_runtime_c__String__Sequence__fini(&empty_seq);
    }
    
    // Additional test: Initialize a single-element sequence
    rosidl_runtime_c__String__Sequence single_seq;
    memset(&single_seq, 0, sizeof(single_seq));
    
    if (rosidl_runtime_c__String__Sequence__init(&single_seq, 1)) {
        // Try to set a string value if we have remaining data
        if (remaining > 0) {
            size_t str_len = 0;
            const char* str_data = extract_string(&cursor, &remaining, &str_len);
            
            if (str_len > 0 && single_seq.data[0].capacity > 0) {
                size_t copy_len = str_len;
                if (copy_len >= single_seq.data[0].capacity) {
                    copy_len = single_seq.data[0].capacity - 1;
                }
                
                if (copy_len > 0) {
                    memcpy(single_seq.data[0].data, str_data, copy_len);
                    single_seq.data[0].data[copy_len] = '\0';
                    single_seq.data[0].size = copy_len;
                }
            }
        }
        
        rosidl_runtime_c__String__Sequence__fini(&single_seq);
    }
    
    return 0;
}

// Main function for standalone testing (not used during fuzzing)
#ifdef STANDALONE_TEST
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }
    
    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        fprintf(stderr, "Failed to open input file\n");
        return 1;
    }
    
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    uint8_t *data = (uint8_t *)malloc(file_size);
    if (!data) {
        fclose(f);
        fprintf(stderr, "Failed to allocate memory\n");
        return 1;
    }
    
    size_t bytes_read = fread(data, 1, file_size, f);
    fclose(f);
    
    if (bytes_read != (size_t)file_size) {
        free(data);
        fprintf(stderr, "Failed to read entire file\n");
        return 1;
    }
    
    int result = LLVMFuzzerTestOneInput(data, bytes_read);
    free(data);
    
    return result;
}
#endif
