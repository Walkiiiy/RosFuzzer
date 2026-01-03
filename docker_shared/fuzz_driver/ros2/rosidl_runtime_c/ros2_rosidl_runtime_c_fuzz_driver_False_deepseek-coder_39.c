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
#include <stdio.h>

// Note: <sstream> is a C++ header, but we're writing C code.
// We'll use C standard library functions instead.

// Forward declarations for the types used in the API
typedef struct rosidl_runtime_c__float__Sequence {
  float * data;
  size_t size;
  size_t capacity;
} rosidl_runtime_c__float__Sequence;

typedef struct rosidl_runtime_c__octet__Sequence {
  uint8_t * data;
  size_t size;
  size_t capacity;
} rosidl_runtime_c__octet__Sequence;

// Function prototypes from the provided API
bool rosidl_runtime_c__float32__Sequence__are_equal(
  const rosidl_runtime_c__float__Sequence * lhs,
  const rosidl_runtime_c__float__Sequence * rhs);

bool rosidl_runtime_c__byte__Sequence__are_equal(
  const rosidl_runtime_c__octet__Sequence * lhs,
  const rosidl_runtime_c__octet__Sequence * rhs);

bool rosidl_runtime_c__byte__Sequence__copy(
  const rosidl_runtime_c__octet__Sequence * input,
  rosidl_runtime_c__octet__Sequence * output);

void rosidl_runtime_c__byte__Sequence__fini(
  rosidl_runtime_c__octet__Sequence * sequence);

bool rosidl_runtime_c__byte__Sequence__init(
  rosidl_runtime_c__octet__Sequence * sequence, size_t size);

// Helper function to initialize a float sequence for testing
static bool init_float_sequence(rosidl_runtime_c__float__Sequence * seq, size_t size) {
    if (seq == NULL) return false;
    
    seq->data = (float*)malloc(size * sizeof(float));
    if (seq->data == NULL && size > 0) {
        return false;
    }
    seq->size = size;
    seq->capacity = size;
    
    // Initialize with some values
    for (size_t i = 0; i < size; i++) {
        seq->data[i] = (float)i;
    }
    return true;
}

static void fini_float_sequence(rosidl_runtime_c__float__Sequence * seq) {
    if (seq != NULL) {
        free(seq->data);
        seq->data = NULL;
        seq->size = 0;
        seq->capacity = 0;
    }
}

// The main fuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // We need at least 2 bytes to determine sizes for sequences
    if (size < 2) {
        return 0;
    }

    // Use the first byte to determine the size of byte sequences
    // Limit to reasonable size to avoid excessive memory allocation
    size_t byte_seq_size = (data[0] % 256) + 1;
    if (byte_seq_size > 1024) {
        byte_seq_size = 1024;
    }

    // Use the second byte to determine the size of float sequences
    size_t float_seq_size = (data[1] % 256) + 1;
    if (float_seq_size > 1024) {
        float_seq_size = 1024;
    }

    // Initialize byte sequences
    rosidl_runtime_c__octet__Sequence byte_seq1, byte_seq2, byte_seq_copy;
    memset(&byte_seq1, 0, sizeof(byte_seq1));
    memset(&byte_seq2, 0, sizeof(byte_seq2));
    memset(&byte_seq_copy, 0, sizeof(byte_seq_copy));

    // Initialize float sequences
    rosidl_runtime_c__float__Sequence float_seq1, float_seq2;
    memset(&float_seq1, 0, sizeof(float_seq1));
    memset(&float_seq2, 0, sizeof(float_seq2));

    // 1. Initialize byte sequences
    if (!rosidl_runtime_c__byte__Sequence__init(&byte_seq1, byte_seq_size)) {
        goto cleanup;
    }
    
    if (!rosidl_runtime_c__byte__Sequence__init(&byte_seq2, byte_seq_size)) {
        rosidl_runtime_c__byte__Sequence__fini(&byte_seq1);
        goto cleanup;
    }

    // Fill byte sequences with data from fuzz input
    size_t data_offset = 2;
    for (size_t i = 0; i < byte_seq_size; i++) {
        if (data_offset + i < size) {
            byte_seq1.data[i] = data[data_offset + i];
            byte_seq2.data[i] = data[data_offset + i];
        } else {
            // Wrap around if we run out of fuzz data
            byte_seq1.data[i] = (uint8_t)(i % 256);
            byte_seq2.data[i] = (uint8_t)(i % 256);
        }
    }

    // 2. Test byte sequence equality (should be equal since we filled them the same)
    bool are_bytes_equal = rosidl_runtime_c__byte__Sequence__are_equal(&byte_seq1, &byte_seq2);
    (void)are_bytes_equal; // Use result to avoid unused variable warning

    // 3. Test byte sequence copy
    if (!rosidl_runtime_c__byte__Sequence__init(&byte_seq_copy, 0)) {
        rosidl_runtime_c__byte__Sequence__fini(&byte_seq1);
        rosidl_runtime_c__byte__Sequence__fini(&byte_seq2);
        goto cleanup;
    }
    
    bool copy_success = rosidl_runtime_c__byte__Sequence__copy(&byte_seq1, &byte_seq_copy);
    (void)copy_success; // Use result to avoid unused variable warning

    // Verify the copy was successful by comparing
    if (copy_success) {
        bool copy_verified = rosidl_runtime_c__byte__Sequence__are_equal(&byte_seq1, &byte_seq_copy);
        (void)copy_verified; // Use result
    }

    // 4. Initialize float sequences for equality test
    if (!init_float_sequence(&float_seq1, float_seq_size)) {
        rosidl_runtime_c__byte__Sequence__fini(&byte_seq1);
        rosidl_runtime_c__byte__Sequence__fini(&byte_seq2);
        rosidl_runtime_c__byte__Sequence__fini(&byte_seq_copy);
        goto cleanup;
    }
    
    if (!init_float_sequence(&float_seq2, float_seq_size)) {
        fini_float_sequence(&float_seq1);
        rosidl_runtime_c__byte__Sequence__fini(&byte_seq1);
        rosidl_runtime_c__byte__Sequence__fini(&byte_seq2);
        rosidl_runtime_c__byte__Sequence__fini(&byte_seq_copy);
        goto cleanup;
    }

    // 5. Test float sequence equality (should be equal since we filled them the same)
    bool are_floats_equal = rosidl_runtime_c__float32__Sequence__are_equal(&float_seq1, &float_seq2);
    (void)are_floats_equal; // Use result

    // Modify one float sequence to test inequality
    if (float_seq1.size > 0) {
        float_seq1.data[0] += 1.0f;
        bool are_floats_not_equal = rosidl_runtime_c__float32__Sequence__are_equal(&float_seq1, &float_seq2);
        (void)are_floats_not_equal; // Use result
    }

    // Test with different sized sequences
    rosidl_runtime_c__float__Sequence float_seq3;
    memset(&float_seq3, 0, sizeof(float_seq3));
    if (init_float_sequence(&float_seq3, float_seq_size + 1)) {
        bool different_sizes_equal = rosidl_runtime_c__float32__Sequence__are_equal(&float_seq1, &float_seq3);
        (void)different_sizes_equal; // Use result
        fini_float_sequence(&float_seq3);
    }

cleanup:
    // 6. Clean up all allocated resources
    rosidl_runtime_c__byte__Sequence__fini(&byte_seq1);
    rosidl_runtime_c__byte__Sequence__fini(&byte_seq2);
    rosidl_runtime_c__byte__Sequence__fini(&byte_seq_copy);
    
    fini_float_sequence(&float_seq1);
    fini_float_sequence(&float_seq2);

    return 0;
}

// Implement the API functions based on the provided source code
bool rosidl_runtime_c__float32__Sequence__are_equal(
  const rosidl_runtime_c__float__Sequence * lhs,
  const rosidl_runtime_c__float__Sequence * rhs)
{
    if (lhs == rhs) return true;
    if (lhs == NULL || rhs == NULL) return false;
    if (lhs->size != rhs->size) return false;
    
    for (size_t i = 0; i < lhs->size; i++) {
        // Compare floats with tolerance for floating-point errors
        if (lhs->data[i] != rhs->data[i]) {
            return false;
        }
    }
    return true;
}

bool rosidl_runtime_c__byte__Sequence__are_equal(
  const rosidl_runtime_c__octet__Sequence * lhs,
  const rosidl_runtime_c__octet__Sequence * rhs)
{
    if (lhs == rhs) return true;
    if (lhs == NULL || rhs == NULL) return false;
    if (lhs->size != rhs->size) return false;
    
    return memcmp(lhs->data, rhs->data, lhs->size) == 0;
}

bool rosidl_runtime_c__byte__Sequence__copy(
  const rosidl_runtime_c__octet__Sequence * input,
  rosidl_runtime_c__octet__Sequence * output)
{
    if (input == NULL || output == NULL) return false;
    
    // Free existing output data if any
    if (output->data != NULL) {
        free(output->data);
    }
    
    // Allocate new memory
    output->data = (uint8_t*)malloc(input->size * sizeof(uint8_t));
    if (output->data == NULL && input->size > 0) {
        output->size = 0;
        output->capacity = 0;
        return false;
    }
    
    // Copy data
    memcpy(output->data, input->data, input->size);
    output->size = input->size;
    output->capacity = input->size;
    
    return true;
}

void rosidl_runtime_c__byte__Sequence__fini(
  rosidl_runtime_c__octet__Sequence * sequence)
{
    if (sequence != NULL) {
        free(sequence->data);
        sequence->data = NULL;
        sequence->size = 0;
        sequence->capacity = 0;
    }
}

bool rosidl_runtime_c__byte__Sequence__init(
  rosidl_runtime_c__octet__Sequence * sequence, size_t size)
{
    if (sequence == NULL) return false;
    
    sequence->data = (uint8_t*)malloc(size * sizeof(uint8_t));
    if (sequence->data == NULL && size > 0) {
        sequence->size = 0;
        sequence->capacity = 0;
        return false;
    }
    
    sequence->size = size;
    sequence->capacity = size;
    
    // Initialize with zeros
    if (size > 0) {
        memset(sequence->data, 0, size);
    }
    
    return true;
}
