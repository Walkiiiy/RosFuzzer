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
#include <stdio.h>

// Include rcutils headers for allocator support
#include <rcutils/allocator.h>

// LLVMFuzzerTestOneInput function signature
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

// Helper function to create a rosidl_runtime_c__String__Sequence
static bool create_string_sequence(rosidl_runtime_c__String__Sequence *seq, 
                                   size_t seq_size, 
                                   const uint8_t *data, 
                                   size_t data_size) {
    if (!seq || seq_size == 0) {
        return false;
    }
    
    // Initialize sequence
    if (!rosidl_runtime_c__String__Sequence__init(seq, seq_size)) {
        return false;
    }
    
    // Fill each string in the sequence with data from the fuzz input
    for (size_t i = 0; i < seq_size && i < seq->size; ++i) {
        // Calculate how much data to assign to this string
        size_t data_offset = i % (data_size + 1); // +1 to avoid modulo by 0
        size_t assign_size = (data_size > 0) ? (data_size - data_offset) : 0;
        
        // Limit assign_size to prevent excessive memory usage
        if (assign_size > 256) {
            assign_size = 256;
        }
        
        // Ensure we don't read beyond data buffer
        if (data_offset >= data_size) {
            assign_size = 0;
        }
        
        // Initialize the string
        if (!rosidl_runtime_c__String__init(&seq->data[i])) {
            rosidl_runtime_c__String__Sequence__fini(seq);
            return false;
        }
        
        // Assign data if available
        if (assign_size > 0 && data_offset < data_size) {
            // Create a temporary buffer with the data to assign
            char *temp_data = (char *)malloc(assign_size + 1);
            if (!temp_data) {
                rosidl_runtime_c__String__Sequence__fini(seq);
                return false;
            }
            
            // Copy data safely
            size_t copy_size = (data_size - data_offset < assign_size) ? 
                              data_size - data_offset : assign_size;
            memcpy(temp_data, data + data_offset, copy_size);
            
            // Fill remaining with pattern if needed
            for (size_t j = copy_size; j < assign_size; j++) {
                temp_data[j] = 'A' + (j % 26);
            }
            temp_data[assign_size] = '\0';
            
            // Assign to string
            if (!rosidl_runtime_c__String__assignn(&seq->data[i], temp_data, assign_size)) {
                free(temp_data);
                rosidl_runtime_c__String__Sequence__fini(seq);
                return false;
            }
            
            free(temp_data);
        }
    }
    
    return true;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Ensure we have minimum data for testing
    if (size < 1) {
        return 0;
    }
    
    // Initialize variables
    rosidl_runtime_c__String str1 = {0};
    rosidl_runtime_c__String str2 = {0};
    rosidl_runtime_c__String__Sequence seq1 = {0};
    rosidl_runtime_c__String__Sequence seq2 = {0};
    
    bool test_passed = true;
    
    // 1. Test rosidl_runtime_c__String__init
    if (!rosidl_runtime_c__String__init(&str1)) {
        // Initialization failed, clean up and return
        goto cleanup;
    }
    
    if (!rosidl_runtime_c__String__init(&str2)) {
        rosidl_runtime_c__String__fini(&str1);
        goto cleanup;
    }
    
    // 2. Test rosidl_runtime_c__String__assignn
    // Use fuzz data for string assignment
    size_t assign_size = size;
    if (assign_size > 1024) {
        assign_size = 1024; // Limit size to prevent excessive memory usage
    }
    
    // Create a null-terminated copy of the data for assignn
    char *temp_data = (char *)malloc(assign_size + 1);
    if (!temp_data) {
        goto cleanup;
    }
    
    // Copy data safely
    memcpy(temp_data, data, assign_size);
    temp_data[assign_size] = '\0';
    
    // Assign to first string
    if (!rosidl_runtime_c__String__assignn(&str1, temp_data, assign_size)) {
        free(temp_data);
        goto cleanup;
    }
    
    // Assign same data to second string for equality test
    if (!rosidl_runtime_c__String__assignn(&str2, temp_data, assign_size)) {
        free(temp_data);
        goto cleanup;
    }
    
    free(temp_data);
    
    // 3. Test rosidl_runtime_c__String__are_equal
    // Strings should be equal since they have the same content
    bool are_equal = rosidl_runtime_c__String__are_equal(&str1, &str2);
    if (!are_equal) {
        // This is unexpected since we assigned the same data
        test_passed = false;
    }
    
    // Test with NULL pointers
    if (rosidl_runtime_c__String__are_equal(NULL, &str1)) {
        test_passed = false;
    }
    
    if (rosidl_runtime_c__String__are_equal(&str1, NULL)) {
        test_passed = false;
    }
    
    if (rosidl_runtime_c__String__are_equal(NULL, NULL)) {
        test_passed = false;
    }
    
    // 4. Test rosidl_runtime_c__String__Sequence__are_equal
    // Create two sequences with similar content
    size_t seq_size = (size % 8) + 1; // Sequence size between 1 and 8
    if (!create_string_sequence(&seq1, seq_size, data, size)) {
        goto cleanup;
    }
    
    // Create second sequence with same content
    if (!create_string_sequence(&seq2, seq_size, data, size)) {
        rosidl_runtime_c__String__Sequence__fini(&seq1);
        goto cleanup;
    }
    
    // Sequences should be equal
    bool seqs_equal = rosidl_runtime_c__String__Sequence__are_equal(&seq1, &seq2);
    if (!seqs_equal) {
        test_passed = false;
    }
    
    // Test with NULL pointers
    if (rosidl_runtime_c__String__Sequence__are_equal(NULL, &seq1)) {
        test_passed = false;
    }
    
    if (rosidl_runtime_c__String__Sequence__are_equal(&seq1, NULL)) {
        test_passed = false;
    }
    
    if (rosidl_runtime_c__String__Sequence__are_equal(NULL, NULL)) {
        test_passed = false;
    }
    
    // 5. Test with modified second sequence to make them unequal
    if (seq2.size > 0) {
        // Modify first string in second sequence
        if (!rosidl_runtime_c__String__assignn(&seq2.data[0], "different", 9)) {
            // If assignment fails, sequences might still be equal
            // This is acceptable for fuzzing
        } else {
            // Now sequences should not be equal
            if (rosidl_runtime_c__String__Sequence__are_equal(&seq1, &seq2)) {
                test_passed = false;
            }
        }
    }

cleanup:
    // 6. Test rosidl_runtime_c__String__fini
    // Clean up all allocated resources in reverse order
    
    // Clean up sequences
    if (seq1.data) {
        rosidl_runtime_c__String__Sequence__fini(&seq1);
    }
    
    if (seq2.data) {
        rosidl_runtime_c__String__Sequence__fini(&seq2);
    }
    
    // Clean up individual strings
    rosidl_runtime_c__String__fini(&str1);
    rosidl_runtime_c__String__fini(&str2);
    
    // Return 0 to continue fuzzing (non-zero would indicate an error to stop fuzzing)
    return 0;
}

// Note: The following functions are referenced but not defined in the provided headers.
// We need to include their declarations or define them if they're missing.
// Based on ROS 2 conventions, we'll add declarations for the missing sequence functions.

#ifndef ROSIDL_RUNTIME_C__STRING_FUNCTIONS_H__
// Forward declarations for sequence functions if not already in headers
bool rosidl_runtime_c__String__Sequence__init(
    rosidl_runtime_c__String__Sequence * sequence, size_t size);
void rosidl_runtime_c__String__Sequence__fini(
    rosidl_runtime_c__String__Sequence * sequence);
#endif

// If the sequence functions are not available in the headers, we need to handle them.
// Since we cannot implement them ourselves (rule #5), we'll assume they exist in the library.
// In a real scenario, these would be provided by the rosidl_runtime_c library.

// Main function for standalone testing (not used during fuzzing)
#ifdef STANDALONE_TEST
#include <stdio.h>
int main() {
    // Simple test with sample data
    uint8_t test_data[] = "Hello, Fuzzer!";
    return LLVMFuzzerTestOneInput(test_data, sizeof(test_data) - 1);
}
#endif
