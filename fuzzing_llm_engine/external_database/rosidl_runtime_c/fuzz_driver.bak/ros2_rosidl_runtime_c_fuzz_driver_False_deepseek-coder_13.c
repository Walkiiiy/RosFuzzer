#include "rosidl_runtime_c/message_type_support.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"
#include "rosidl_runtime_c/sequence_bound.h"
#include "rosidl_runtime_c/service_type_support.h"
#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_runtime_c/u16string_functions.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sstream>

// Forward declarations for types used in the API
typedef struct rosidl_runtime_c__String__Sequence
{
    rosidl_runtime_c__String * data;
    size_t size;
    size_t capacity;
} rosidl_runtime_c__String__Sequence;

typedef struct rosidl_runtime_c__boolean__Sequence
{
    bool * data;
    size_t size;
    size_t capacity;
} rosidl_runtime_c__boolean__Sequence;

// Function prototypes from the provided API
bool rosidl_runtime_c__String__Sequence__are_equal(
    const rosidl_runtime_c__String__Sequence * lhs,
    const rosidl_runtime_c__String__Sequence * rhs);

bool rosidl_runtime_c__bool__Sequence__are_equal(
    const rosidl_runtime_c__boolean__Sequence * lhs,
    const rosidl_runtime_c__boolean__Sequence * rhs);

bool rosidl_runtime_c__String__init(rosidl_runtime_c__String * str);

bool rosidl_runtime_c__String__are_equal(
    const rosidl_runtime_c__String * lhs,
    const rosidl_runtime_c__String * rhs);

void rosidl_runtime_c__String__fini(rosidl_runtime_c__String * str);

// Helper function to create a string sequence from fuzz data
static bool create_string_sequence_from_fuzz(
    const uint8_t *data, 
    size_t size, 
    rosidl_runtime_c__String__Sequence *seq,
    size_t max_strings)
{
    if (!seq || max_strings == 0) {
        return false;
    }
    
    // Initialize sequence
    seq->data = (rosidl_runtime_c__String*)calloc(max_strings, sizeof(rosidl_runtime_c__String));
    if (!seq->data) {
        return false;
    }
    
    seq->size = 0;
    seq->capacity = max_strings;
    
    // Create strings from fuzz data
    size_t data_index = 0;
    for (size_t i = 0; i < max_strings && data_index < size; i++) {
        if (!rosidl_runtime_c__String__init(&seq->data[i])) {
            // Clean up already initialized strings
            for (size_t j = 0; j < i; j++) {
                rosidl_runtime_c__String__fini(&seq->data[j]);
            }
            free(seq->data);
            seq->data = NULL;
            return false;
        }
        
        // Determine string length (1-255 bytes, limited by remaining data)
        size_t str_len = 1;
        if (size - data_index > 1) {
            str_len = (data[data_index] % 255) + 1;
            if (str_len > size - data_index - 1) {
                str_len = size - data_index - 1;
            }
        }
        
        if (str_len > 0) {
            // Reallocate to fit the string
            free(seq->data[i].data);
            seq->data[i].data = (char*)malloc(str_len + 1);
            if (!seq->data[i].data) {
                rosidl_runtime_c__String__fini(&seq->data[i]);
                for (size_t j = 0; j < i; j++) {
                    rosidl_runtime_c__String__fini(&seq->data[j]);
                }
                free(seq->data);
                seq->data = NULL;
                return false;
            }
            
            // Copy data and null-terminate
            memcpy(seq->data[i].data, &data[data_index + 1], str_len);
            seq->data[i].data[str_len] = '\0';
            seq->data[i].size = str_len;
            seq->data[i].capacity = str_len + 1;
            
            data_index += str_len + 1;
        }
        
        seq->size++;
    }
    
    return true;
}

// Helper function to create a boolean sequence from fuzz data
static bool create_bool_sequence_from_fuzz(
    const uint8_t *data, 
    size_t size, 
    rosidl_runtime_c__boolean__Sequence *seq,
    size_t max_bools)
{
    if (!seq || max_bools == 0) {
        return false;
    }
    
    // Limit max_bools to available data
    if (max_bools > size) {
        max_bools = size;
    }
    
    seq->data = (bool*)calloc(max_bools, sizeof(bool));
    if (!seq->data) {
        return false;
    }
    
    seq->size = max_bools;
    seq->capacity = max_bools;
    
    // Fill with fuzz data
    for (size_t i = 0; i < max_bools; i++) {
        seq->data[i] = (data[i] & 0x01) != 0;
    }
    
    return true;
}

// Helper function to free string sequence
static void free_string_sequence(rosidl_runtime_c__String__Sequence *seq)
{
    if (!seq || !seq->data) {
        return;
    }
    
    for (size_t i = 0; i < seq->size; i++) {
        rosidl_runtime_c__String__fini(&seq->data[i]);
    }
    
    free(seq->data);
    seq->data = NULL;
    seq->size = 0;
    seq->capacity = 0;
}

// Helper function to free boolean sequence
static void free_bool_sequence(rosidl_runtime_c__boolean__Sequence *seq)
{
    if (!seq || !seq->data) {
        return;
    }
    
    free(seq->data);
    seq->data = NULL;
    seq->size = 0;
    seq->capacity = 0;
}

// Main fuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    // Skip if no data
    if (!data || size == 0) {
        return 0;
    }
    
    // Initialize individual strings for testing rosidl_runtime_c__String__are_equal
    rosidl_runtime_c__String str1, str2;
    bool init_success = false;
    
    // Initialize first string
    if (!rosidl_runtime_c__String__init(&str1)) {
        goto cleanup;
    }
    
    // Initialize second string
    if (!rosidl_runtime_c__String__init(&str2)) {
        rosidl_runtime_c__String__fini(&str1);
        goto cleanup;
    }
    
    init_success = true;
    
    // Test string equality with empty strings
    bool are_equal = rosidl_runtime_c__String__are_equal(&str1, &str2);
    (void)are_equal; // Result intentionally unused
    
    // Create non-empty strings from fuzz data if we have enough data
    if (size >= 2) {
        // Free existing allocations
        rosidl_runtime_c__String__fini(&str1);
        rosidl_runtime_c__String__fini(&str2);
        
        // Reinitialize
        if (!rosidl_runtime_c__String__init(&str1) || 
            !rosidl_runtime_c__String__init(&str2)) {
            goto cleanup;
        }
        
        // Create strings from fuzz data
        size_t str1_len = (data[0] % 64) + 1;
        if (str1_len > size - 1) {
            str1_len = size - 1;
        }
        
        if (str1_len > 0) {
            free(str1.data);
            str1.data = (char*)malloc(str1_len + 1);
            if (str1.data) {
                memcpy(str1.data, &data[1], str1_len);
                str1.data[str1_len] = '\0';
                str1.size = str1_len;
                str1.capacity = str1_len + 1;
            }
        }
        
        // Test equality again
        are_equal = rosidl_runtime_c__String__are_equal(&str1, &str2);
        (void)are_equal;
    }
    
    // Create string sequences for testing rosidl_runtime_c__String__Sequence__are_equal
    rosidl_runtime_c__String__Sequence seq1 = {0};
    rosidl_runtime_c__String__Sequence seq2 = {0};
    
    // Create sequences from fuzz data (limited to 4 strings each to avoid excessive memory)
    if (create_string_sequence_from_fuzz(data, size, &seq1, 4)) {
        // Create a second sequence - sometimes identical, sometimes different
        if (create_string_sequence_from_fuzz(data, size, &seq2, 4)) {
            // Test sequence equality
            bool seqs_equal = rosidl_runtime_c__String__Sequence__are_equal(&seq1, &seq2);
            (void)seqs_equal;
            
            // Also test with NULL pointers
            seqs_equal = rosidl_runtime_c__String__Sequence__are_equal(NULL, &seq1);
            seqs_equal = rosidl_runtime_c__String__Sequence__are_equal(&seq2, NULL);
            seqs_equal = rosidl_runtime_c__String__Sequence__are_equal(NULL, NULL);
        }
        
        // Clean up sequences
        free_string_sequence(&seq1);
        free_string_sequence(&seq2);
    }
    
    // Create boolean sequences for testing rosidl_runtime_c__bool__Sequence__are_equal
    rosidl_runtime_c__boolean__Sequence bool_seq1 = {0};
    rosidl_runtime_c__boolean__Sequence bool_seq2 = {0};
    
    // Create boolean sequences from fuzz data
    if (create_bool_sequence_from_fuzz(data, size, &bool_seq1, 8)) {
        // Create second boolean sequence
        if (create_bool_sequence_from_fuzz(data, size, &bool_seq2, 8)) {
            // Test boolean sequence equality
            bool bools_equal = rosidl_runtime_c__bool__Sequence__are_equal(&bool_seq1, &bool_seq2);
            (void)bools_equal;
            
            // Test with potentially equal sequences
            if (bool_seq1.size == bool_seq2.size && bool_seq1.size > 0) {
                // Make them equal for testing
                memcpy(bool_seq2.data, bool_seq1.data, bool_seq1.size * sizeof(bool));
                bools_equal = rosidl_runtime_c__bool__Sequence__are_equal(&bool_seq1, &bool_seq2);
                (void)bools_equal;
            }
        }
        
        // Clean up boolean sequences
        free_bool_sequence(&bool_seq1);
        free_bool_sequence(&bool_seq2);
    }
    
cleanup:
    // Clean up individual strings
    if (init_success) {
        rosidl_runtime_c__String__fini(&str1);
        rosidl_runtime_c__String__fini(&str2);
    }
    
    return 0;
}
