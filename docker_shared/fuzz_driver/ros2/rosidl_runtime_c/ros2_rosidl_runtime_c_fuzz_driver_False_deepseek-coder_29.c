#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sstream>
#include "rosidl_runtime_c/message_type_support.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"
#include "rosidl_runtime_c/sequence_bound.h"
#include "rosidl_runtime_c/service_type_support.h"
#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_runtime_c/u16string_functions.h"

// Helper function to extract a uint16_t value from fuzz data
static uint16_t get_u16_from_data(const uint8_t **data, size_t *size) {
    if (*size < sizeof(uint16_t)) {
        return 0;
    }
    uint16_t value;
    memcpy(&value, *data, sizeof(uint16_t));
    *data += sizeof(uint16_t);
    *size -= sizeof(uint16_t);
    return value;
}

// Helper function to extract size_t value from fuzz data
static size_t get_size_from_data(const uint8_t **data, size_t *size) {
    if (*size < sizeof(size_t)) {
        return 0;
    }
    size_t value;
    memcpy(&value, *data, sizeof(size_t));
    *data += sizeof(size_t);
    *size -= sizeof(size_t);
    return value;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early exit if we don't have enough data for basic operations
    if (size < sizeof(size_t) + sizeof(uint16_t)) {
        return 0;
    }

    // Initialize variables
    rosidl_runtime_c__U16String str1 = {0};
    rosidl_runtime_c__U16String str2 = {0};
    rosidl_runtime_c__U16String str3 = {0};
    rosidl_runtime_c__U16String__Sequence seq1 = {0};
    rosidl_runtime_c__U16String__Sequence seq2 = {0};
    
    // Keep track of what we've initialized to clean up properly
    bool str1_initialized = false;
    bool str2_initialized = false;
    bool str3_initialized = false;
    bool seq1_initialized = false;
    bool seq2_initialized = false;

    // Create a mutable copy of the input pointers for safe parsing
    const uint8_t *current_data = data;
    size_t remaining_size = size;

    // 1. Initialize str1 using rosidl_runtime_c__U16String__init
    if (!rosidl_runtime_c__U16String__init(&str1)) {
        goto cleanup;
    }
    str1_initialized = true;

    // 2. Initialize str2
    if (!rosidl_runtime_c__U16String__init(&str2)) {
        goto cleanup;
    }
    str2_initialized = true;

    // 3. Initialize str3
    if (!rosidl_runtime_c__U16String__init(&str3)) {
        goto cleanup;
    }
    str3_initialized = true;

    // 4. Create some UTF-16 content from fuzz data for str1
    // Determine how many uint16_t characters we can safely extract
    size_t max_chars = remaining_size / sizeof(uint16_t);
    if (max_chars > 0) {
        // Limit to reasonable size to avoid excessive memory usage
        size_t num_chars = max_chars % 256;  // Cap at 255 characters
        
        // Allocate enough space for the characters plus null terminator
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        uint16_t *temp_data = (uint16_t *)allocator.allocate(
            (num_chars + 1) * sizeof(uint16_t), allocator.state);
        
        if (temp_data) {
            // Copy UTF-16 characters from fuzz data
            for (size_t i = 0; i < num_chars && remaining_size >= sizeof(uint16_t); i++) {
                temp_data[i] = get_u16_from_data(&current_data, &remaining_size);
            }
            temp_data[num_chars] = 0;  // Null terminate
            
            // Assign to str1 using assignn (which is what copy uses internally)
            // Note: We're simulating what rosidl_runtime_c__U16String__copy would do
            if (!rosidl_runtime_c__U16String__assignn(&str1, temp_data, num_chars)) {
                allocator.deallocate(temp_data, allocator.state);
                goto cleanup;
            }
            allocator.deallocate(temp_data, allocator.state);
        }
    }

    // 5. Copy str1 to str2 using rosidl_runtime_c__U16String__copy
    if (!rosidl_runtime_c__U16String__copy(&str1, &str2)) {
        goto cleanup;
    }

    // 6. Compare str1 and str2 using rosidl_runtime_c__U16String__are_equal
    // They should be equal since we just copied
    bool are_equal = rosidl_runtime_c__U16String__are_equal(&str1, &str2);
    if (!are_equal) {
        // This is unexpected but not an error - just continue fuzzing
        // In a real fuzzer, we might want to log this
    }

    // 7. Initialize sequences for sequence copy test
    // Determine sequence size from fuzz data (capped for safety)
    size_t seq_size = get_size_from_data(&current_data, &remaining_size) % 8;
    
    // Initialize seq1
    seq1.size = seq_size;
    seq1.capacity = seq_size;
    if (seq_size > 0) {
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        seq1.data = (rosidl_runtime_c__U16String *)allocator.allocate(
            seq_size * sizeof(rosidl_runtime_c__U16String), allocator.state);
        
        if (!seq1.data) {
            goto cleanup;
        }
        
        // Initialize each string in the sequence
        for (size_t i = 0; i < seq_size; i++) {
            if (!rosidl_runtime_c__U16String__init(&seq1.data[i])) {
                // Clean up already initialized strings
                for (size_t j = 0; j < i; j++) {
                    rosidl_runtime_c__U16String__fini(&seq1.data[j]);
                }
                allocator.deallocate(seq1.data, allocator.state);
                goto cleanup;
            }
            
            // Add some simple content to each string
            uint16_t simple_content[2] = { (uint16_t)('A' + (i % 26)), 0 };
            if (!rosidl_runtime_c__U16String__assignn(&seq1.data[i], simple_content, 1)) {
                // Clean up on failure
                for (size_t j = 0; j <= i; j++) {
                    rosidl_runtime_c__U16String__fini(&seq1.data[j]);
                }
                allocator.deallocate(seq1.data, allocator.state);
                goto cleanup;
            }
        }
        seq1_initialized = true;
    }

    // 8. Initialize seq2 with zero capacity
    seq2.size = 0;
    seq2.capacity = 0;
    seq2.data = NULL;
    seq2_initialized = true;  // Mark as initialized even if empty

    // 9. Copy sequence using rosidl_runtime_c__U16String__Sequence__copy
    if (!rosidl_runtime_c__U16String__Sequence__copy(&seq1, &seq2)) {
        // Copy might fail due to memory allocation - that's OK for fuzzing
        // We'll just continue with cleanup
    }

    // 10. Test equality on some of the copied strings (if copy succeeded)
    if (seq2.size > 0 && seq2.data) {
        for (size_t i = 0; i < seq2.size && i < seq1.size; i++) {
            bool seq_strings_equal = rosidl_runtime_c__U16String__are_equal(
                &seq1.data[i], &seq2.data[i]);
            // Result can be true or false depending on copy success
            // Both are valid outcomes for fuzzing
            (void)seq_strings_equal;  // Suppress unused variable warning
        }
    }

    // 11. Test copy with NULL inputs (should return false)
    bool null_test1 = rosidl_runtime_c__U16String__copy(NULL, &str3);
    bool null_test2 = rosidl_runtime_c__U16String__are_equal(NULL, &str1);
    bool null_test3 = rosidl_runtime_c__U16String__are_equal(&str1, NULL);
    bool null_test4 = rosidl_runtime_c__U16String__Sequence__copy(NULL, &seq2);
    bool null_test5 = rosidl_runtime_c__U16String__Sequence__copy(&seq1, NULL);
    
    // All of these should return false, but we don't assert in fuzzing
    (void)null_test1;
    (void)null_test2;
    (void)null_test3;
    (void)null_test4;
    (void)null_test5;

cleanup:
    // Clean up all allocated resources in reverse order
    
    // Clean up seq2 if it was successfully copied into
    if (seq2_initialized && seq2.data) {
        for (size_t i = 0; i < seq2.size; i++) {
            rosidl_runtime_c__U16String__fini(&seq2.data[i]);
        }
        if (seq2.capacity > 0) {
            rcutils_allocator_t allocator = rcutils_get_default_allocator();
            allocator.deallocate(seq2.data, allocator.state);
        }
    }
    
    // Clean up seq1
    if (seq1_initialized && seq1.data) {
        for (size_t i = 0; i < seq1.size; i++) {
            rosidl_runtime_c__U16String__fini(&seq1.data[i]);
        }
        if (seq1.capacity > 0) {
            rcutils_allocator_t allocator = rcutils_get_default_allocator();
            allocator.deallocate(seq1.data, allocator.state);
        }
    }
    
    // Clean up individual strings
    if (str3_initialized) {
        rosidl_runtime_c__U16String__fini(&str3);
    }
    if (str2_initialized) {
        rosidl_runtime_c__U16String__fini(&str2);
    }
    if (str1_initialized) {
        rosidl_runtime_c__U16String__fini(&str1);
    }

    return 0;
}
