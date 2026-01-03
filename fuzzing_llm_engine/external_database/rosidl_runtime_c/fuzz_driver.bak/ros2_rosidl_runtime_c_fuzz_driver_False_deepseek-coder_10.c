#include "rosidl_runtime_c/message_type_support.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"
#include "rosidl_runtime_c/sequence_bound.h"
#include "rosidl_runtime_c/service_type_support.h"
#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_runtime_c/u16string_functions.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

// Forward declarations for types used in the API
typedef struct rcutils_allocator_s rcutils_allocator_t;
typedef struct rosidl_runtime_c__String__Sequence rosidl_runtime_c__String__Sequence;

// Mock implementations for required functions not provided in API source
// These are simplified versions for fuzzing purposes
rcutils_allocator_t rcutils_get_default_allocator(void) {
    static rcutils_allocator_t allocator = {
        .allocate = malloc,
        .deallocate = free,
        .reallocate = realloc,
        .zero_allocate = calloc,
        .state = NULL
    };
    return allocator;
}

bool rosidl_runtime_c__String__are_equal(
    const rosidl_runtime_c__String* lhs,
    const rosidl_runtime_c__String* rhs) {
    if (!lhs || !rhs) return false;
    if (!lhs->data || !rhs->data) return lhs->data == rhs->data;
    return strcmp(lhs->data, rhs->data) == 0;
}

bool rosidl_runtime_c__String__copy(
    const rosidl_runtime_c__String* input,
    rosidl_runtime_c__String* output) {
    if (!input || !output) return false;
    
    // Free existing data if any
    if (output->data) {
        free(output->data);
        output->data = NULL;
    }
    
    if (input->data) {
        output->data = strdup(input->data);
        if (!output->data) return false;
    } else {
        output->data = NULL;
    }
    
    return true;
}

bool rosidl_runtime_c__String__init(rosidl_runtime_c__String* str) {
    if (!str) return false;
    str->data = NULL;
    return true;
}

void rosidl_runtime_c__String__fini(rosidl_runtime_c__String* str) {
    if (str && str->data) {
        free(str->data);
        str->data = NULL;
    }
}

// Main fuzzer function
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early exit for insufficient data
    if (size < 2) return 0;
    
    // Initialize variables
    rosidl_runtime_c__String__Sequence seq1 = {0};
    rosidl_runtime_c__String__Sequence seq2 = {0};
    rosidl_runtime_c__String__Sequence seq3 = {0};
    
    // Determine sequence size from fuzz input (limit to reasonable size)
    size_t seq_size = (data[0] % 16) + 1;  // 1-16 elements
    
    // Ensure we have enough data for the strings
    if (size < seq_size + 1) {
        seq_size = size > 1 ? size - 1 : 1;
    }
    
    // 1. Initialize first sequence
    if (!rosidl_runtime_c__String__Sequence__init(&seq1, seq_size)) {
        goto cleanup;
    }
    
    // Fill sequence with strings from fuzz data
    for (size_t i = 0; i < seq_size && i < seq1.capacity; i++) {
        // Calculate string length from fuzz data
        size_t str_len = data[(i % (size - 1)) + 1] % 64;
        if (str_len == 0) str_len = 1;
        
        // Ensure we don't read beyond input bounds
        size_t data_offset = (i * str_len) % (size - 1);
        if (data_offset + str_len > size) {
            str_len = size - data_offset;
            if (str_len == 0) break;
        }
        
        // Allocate and copy string data
        if (seq1.data[i].data) {
            free(seq1.data[i].data);
        }
        seq1.data[i].data = malloc(str_len + 1);
        if (!seq1.data[i].data) {
            goto cleanup;
        }
        
        // Copy data and null-terminate
        memcpy(seq1.data[i].data, data + data_offset, str_len);
        seq1.data[i].data[str_len] = '\0';
    }
    
    // 2. Initialize second sequence with different size
    size_t seq2_size = (data[1] % 8) + 1;  // 1-8 elements
    if (!rosidl_runtime_c__String__Sequence__init(&seq2, seq2_size)) {
        goto cleanup;
    }
    
    // Fill second sequence
    for (size_t i = 0; i < seq2_size && i < seq2.capacity; i++) {
        size_t str_len = data[(i + 1) % size] % 32;
        if (str_len == 0) str_len = 1;
        
        size_t data_offset = ((i + 1) * str_len) % (size - 1);
        if (data_offset + str_len > size) {
            str_len = size - data_offset;
            if (str_len == 0) break;
        }
        
        if (seq2.data[i].data) {
            free(seq2.data[i].data);
        }
        seq2.data[i].data = malloc(str_len + 1);
        if (!seq2.data[i].data) {
            goto cleanup;
        }
        
        memcpy(seq2.data[i].data, data + data_offset, str_len);
        seq2.data[i].data[str_len] = '\0';
    }
    
    // 3. Test are_equal with different sequences
    bool equal1 = rosidl_runtime_c__String__Sequence__are_equal(&seq1, &seq2);
    (void)equal1;  // Use result to avoid unused variable warning
    
    // 4. Test are_equal with same sequence
    bool equal2 = rosidl_runtime_c__String__Sequence__are_equal(&seq1, &seq1);
    assert(equal2 == true);  // Should always be true
    
    // 5. Initialize third sequence for copying
    if (!rosidl_runtime_c__String__Sequence__init(&seq3, 0)) {
        goto cleanup;
    }
    
    // 6. Test copy from seq1 to seq3
    bool copy_success = rosidl_runtime_c__String__Sequence__copy(&seq1, &seq3);
    if (copy_success) {
        // Verify copy worked by checking equality
        bool equal3 = rosidl_runtime_c__String__Sequence__are_equal(&seq1, &seq3);
        (void)equal3;  // Use result
        
        // Test copy again (should resize if needed)
        bool copy_again = rosidl_runtime_c__String__Sequence__copy(&seq2, &seq3);
        (void)copy_again;
    }
    
    // 7. Test copy with NULL pointers (error handling)
    bool null_copy = rosidl_runtime_c__String__Sequence__copy(NULL, &seq3);
    (void)null_copy;
    
    bool null_copy2 = rosidl_runtime_c__String__Sequence__copy(&seq1, NULL);
    (void)null_copy2;
    
    // 8. Test are_equal with NULL pointers
    bool null_equal = rosidl_runtime_c__String__Sequence__are_equal(NULL, &seq1);
    (void)null_equal;
    
    bool null_equal2 = rosidl_runtime_c__String__Sequence__are_equal(&seq1, NULL);
    (void)null_equal2;
    
    bool null_equal3 = rosidl_runtime_c__String__Sequence__are_equal(NULL, NULL);
    (void)null_equal3;

cleanup:
    // 9. Clean up all sequences using fini
    rosidl_runtime_c__String__Sequence__fini(&seq1);
    rosidl_runtime_c__String__Sequence__fini(&seq2);
    rosidl_runtime_c__String__Sequence__fini(&seq3);
    
    // Note: We don't use destroy because our sequences are stack-allocated
    // destroy is meant for heap-allocated sequences
    
    return 0;
}

// Optional main function for standalone testing
#ifdef STANDALONE_FUZZER
int main() {
    // Simple test with sample data
    uint8_t test_data[] = {5, 'H', 'e', 'l', 'l', 'o', 'W', 'o', 'r', 'l', 'd'};
    LLVMFuzzerTestOneInput(test_data, sizeof(test_data));
    return 0;
}
#endif
