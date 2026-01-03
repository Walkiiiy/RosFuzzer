#include "rosidl_runtime_c/message_type_support.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"
#include "rosidl_runtime_c/sequence_bound.h"
#include "rosidl_runtime_c/service_type_support.h"
#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_runtime_c/u16string_functions.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Note: <sstream> is a C++ header, but we're writing C code
// We'll use C standard library functions instead

// Forward declarations for the opaque types used in the API
typedef struct rosidl_runtime_c__double__Sequence rosidl_runtime_c__double__Sequence;
typedef struct rosidl_runtime_c__octet__Sequence rosidl_runtime_c__octet__Sequence;

// Function prototypes from the provided API
void rosidl_runtime_c__float64__Sequence__fini(rosidl_runtime_c__double__Sequence * sequence);
bool rosidl_runtime_c__byte__Sequence__are_equal(const rosidl_runtime_c__octet__Sequence * lhs, const rosidl_runtime_c__octet__Sequence * rhs);
bool rosidl_runtime_c__byte__Sequence__copy(const rosidl_runtime_c__octet__Sequence * input, rosidl_runtime_c__octet__Sequence * output);
void rosidl_runtime_c__byte__Sequence__fini(rosidl_runtime_c__octet__Sequence * sequence);
bool rosidl_runtime_c__byte__Sequence__init(rosidl_runtime_c__octet__Sequence * sequence, size_t size);

// Helper function to extract size_t from fuzz data safely
static size_t extract_size_t(const uint8_t *data, size_t size, size_t *offset) {
    if (*offset + sizeof(size_t) > size) {
        // Not enough data, return a small default value
        return 1;
    }
    
    size_t value;
    memcpy(&value, data + *offset, sizeof(size_t));
    *offset += sizeof(size_t);
    
    // Ensure the value is reasonable to avoid excessive memory allocation
    if (value > 1024) {
        value = 1024;
    }
    
    // Ensure at least 1 element for valid sequence
    if (value == 0) {
        value = 1;
    }
    
    return value;
}

// Helper function to extract a byte from fuzz data
static uint8_t extract_byte(const uint8_t *data, size_t size, size_t *offset) {
    if (*offset >= size) {
        return 0;
    }
    return data[(*offset)++];
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early return if we don't have enough data for basic operations
    if (size < sizeof(size_t) * 2) {
        return 0;
    }
    
    size_t offset = 0;
    
    // Initialize two byte sequences for testing
    rosidl_runtime_c__octet__Sequence seq1;
    rosidl_runtime_c__octet__Sequence seq2;
    rosidl_runtime_c__octet__Sequence seq_copy;
    
    // Initialize seq_copy to zero for safety
    memset(&seq_copy, 0, sizeof(seq_copy));
    
    // Extract sizes for the sequences
    size_t size1 = extract_size_t(data, size, &offset);
    size_t size2 = extract_size_t(data, size, &offset);
    
    // Initialize first byte sequence
    if (!rosidl_runtime_c__byte__Sequence__init(&seq1, size1)) {
        // Initialization failed, nothing to clean up
        return 0;
    }
    
    // Initialize second byte sequence
    if (!rosidl_runtime_c__byte__Sequence__init(&seq2, size2)) {
        // Clean up seq1 before returning
        rosidl_runtime_c__byte__Sequence__fini(&seq1);
        return 0;
    }
    
    // Fill sequences with data from fuzz input
    for (size_t i = 0; i < size1 && offset < size; i++) {
        if (seq1.data) {
            seq1.data[i] = extract_byte(data, size, &offset);
        }
    }
    
    // Reset offset for seq2 to get different data pattern
    // We'll use a different part of the input data
    size_t offset2 = size / 2;
    if (offset2 >= size) {
        offset2 = 0;
    }
    
    for (size_t i = 0; i < size2 && offset2 < size; i++) {
        if (seq2.data) {
            seq2.data[i] = data[offset2++];
        }
    }
    
    // Test sequence equality
    bool are_equal = rosidl_runtime_c__byte__Sequence__are_equal(&seq1, &seq2);
    (void)are_equal; // Use the result to avoid unused variable warning
    
    // Test sequence copy
    // First initialize the destination sequence
    if (rosidl_runtime_c__byte__Sequence__init(&seq_copy, size1)) {
        bool copy_success = rosidl_runtime_c__byte__Sequence__copy(&seq1, &seq_copy);
        (void)copy_success; // Use the result
        
        // Test equality again with the copy
        bool copy_equal = rosidl_runtime_c__byte__Sequence__are_equal(&seq1, &seq_copy);
        (void)copy_equal;
        
        // Clean up the copied sequence
        rosidl_runtime_c__byte__Sequence__fini(&seq_copy);
    }
    
    // Create and test a double sequence for the float64 API
    // Note: We need to create a proper double sequence structure
    // Since we don't have the exact definition, we'll simulate it
    // with a simple allocation
    
    // Allocate memory for a double sequence structure
    rosidl_runtime_c__double__Sequence *double_seq = 
        (rosidl_runtime_c__double__Sequence*)malloc(sizeof(rosidl_runtime_c__double__Sequence));
    
    if (double_seq != NULL) {
        // Initialize the structure fields
        // Based on typical ROS 2 sequence structure
        double_seq->data = NULL;
        double_seq->size = 0;
        double_seq->capacity = 0;
        
        // Test the float64 sequence fini function
        rosidl_runtime_c__float64__Sequence__fini(double_seq);
        
        free(double_seq);
    }
    
    // Clean up the byte sequences
    rosidl_runtime_c__byte__Sequence__fini(&seq1);
    rosidl_runtime_c__byte__Sequence__fini(&seq2);
    
    return 0;
}

// If the actual function implementations are not available at link time,
// we need to provide stub implementations. However, since the API source
// code was provided, we assume these functions will be linked from a library.

// In case the functions are not available, here are minimal stub implementations
// that would allow compilation (though they won't provide meaningful fuzzing):

/*
// Stub implementations for compilation without the actual library
void rosidl_runtime_c__float64__Sequence__fini(rosidl_runtime_c__double__Sequence * sequence) {
    if (sequence && sequence->data) {
        free(sequence->data);
        sequence->data = NULL;
        sequence->size = 0;
        sequence->capacity = 0;
    }
}

bool rosidl_runtime_c__byte__Sequence__are_equal(const rosidl_runtime_c__octet__Sequence * lhs,
                                                const rosidl_runtime_c__octet__Sequence * rhs) {
    if (!lhs || !rhs) return false;
    if (lhs->size != rhs->size) return false;
    if (lhs->data == rhs->data) return true;
    if (!lhs->data || !rhs->data) return false;
    
    return memcmp(lhs->data, rhs->data, lhs->size) == 0;
}

bool rosidl_runtime_c__byte__Sequence__copy(const rosidl_runtime_c__octet__Sequence * input,
                                           rosidl_runtime_c__octet__Sequence * output) {
    if (!input || !output || !output->data) return false;
    if (output->capacity < input->size) return false;
    
    memcpy(output->data, input->data, input->size);
    output->size = input->size;
    return true;
}

void rosidl_runtime_c__byte__Sequence__fini(rosidl_runtime_c__octet__Sequence * sequence) {
    if (sequence && sequence->data) {
        free(sequence->data);
        sequence->data = NULL;
        sequence->size = 0;
        sequence->capacity = 0;
    }
}

bool rosidl_runtime_c__byte__Sequence__init(rosidl_runtime_c__octet__Sequence * sequence, size_t size) {
    if (!sequence) return false;
    
    sequence->data = (uint8_t*)calloc(size, sizeof(uint8_t));
    if (!sequence->data && size > 0) return false;
    
    sequence->size = size;
    sequence->capacity = size;
    return true;
}
*/
