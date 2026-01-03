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

// Forward declarations for the actual types used in the API
typedef struct rosidl_runtime_c__boolean__Sequence
{
    bool * data;
    size_t size;
    size_t capacity;
} rosidl_runtime_c__boolean__Sequence;

typedef struct rosidl_runtime_c__octet__Sequence
{
    uint8_t * data;
    size_t size;
    size_t capacity;
} rosidl_runtime_c__octet__Sequence;

// API function declarations (as provided in source code)
bool rosidl_runtime_c__bool__Sequence__are_equal(
    const rosidl_runtime_c__boolean__Sequence * lhs,
    const rosidl_runtime_c__boolean__Sequence * rhs);

bool rosidl_runtime_c__bool__Sequence__init(
    rosidl_runtime_c__boolean__Sequence * sequence, size_t size);

bool rosidl_runtime_c__byte__Sequence__copy(
    const rosidl_runtime_c__octet__Sequence * input,
    rosidl_runtime_c__octet__Sequence * output);

bool rosidl_runtime_c__bool__Sequence__copy(
    const rosidl_runtime_c__boolean__Sequence * input,
    rosidl_runtime_c__boolean__Sequence * output);

void rosidl_runtime_c__bool__Sequence__fini(
    rosidl_runtime_c__boolean__Sequence * sequence);

// Fuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early exit if insufficient data
    if (size < 2) {
        return 0;
    }

    // Initialize all sequences to zero/null
    rosidl_runtime_c__boolean__Sequence bool_seq1 = {0};
    rosidl_runtime_c__boolean__Sequence bool_seq2 = {0};
    rosidl_runtime_c__boolean__Sequence bool_seq_copy = {0};
    rosidl_runtime_c__octet__Sequence byte_seq1 = {0};
    rosidl_runtime_c__octet__Sequence byte_seq_copy = {0};

    // Use first byte to determine sizes (with bounds)
    uint8_t size_mod = data[0];
    size_t bool_size = (size_mod % 64) + 1;  // 1-64 elements
    size_t byte_size = ((data[0] + data[1]) % 128) + 1;  // 1-128 elements

    // Ensure we have enough data for initialization
    if (size < bool_size + byte_size + 2) {
        return 0;
    }

    // 1. Initialize boolean sequence 1
    if (!rosidl_runtime_c__bool__Sequence__init(&bool_seq1, bool_size)) {
        goto cleanup;
    }

    // Fill boolean sequence with data from fuzz input
    for (size_t i = 0; i < bool_size; i++) {
        if (i + 2 < size) {
            bool_seq1.data[i] = (data[i + 2] & 0x01) != 0;
        } else {
            bool_seq1.data[i] = false;
        }
    }

    // 2. Initialize boolean sequence 2 with same size
    if (!rosidl_runtime_c__bool__Sequence__init(&bool_seq2, bool_size)) {
        goto cleanup;
    }

    // Fill boolean sequence 2 with different data pattern
    for (size_t i = 0; i < bool_size; i++) {
        if (i + 2 + bool_size < size) {
            bool_seq2.data[i] = (data[i + 2 + bool_size] & 0x02) != 0;
        } else {
            bool_seq2.data[i] = true;
        }
    }

    // 3. Test equality function
    bool are_equal = rosidl_runtime_c__bool__Sequence__are_equal(&bool_seq1, &bool_seq2);
    (void)are_equal;  // Use result to avoid unused variable warning

    // 4. Copy boolean sequence
    if (!rosidl_runtime_c__bool__Sequence__copy(&bool_seq1, &bool_seq_copy)) {
        goto cleanup;
    }

    // Verify the copy worked by comparing
    bool copy_equal = rosidl_runtime_c__bool__Sequence__are_equal(&bool_seq1, &bool_seq_copy);
    (void)copy_equal;

    // 5. Initialize byte sequence
    byte_seq1.data = (uint8_t*)malloc(byte_size * sizeof(uint8_t));
    if (byte_seq1.data == NULL) {
        goto cleanup;
    }
    byte_seq1.size = byte_size;
    byte_seq1.capacity = byte_size;

    // Fill byte sequence with data
    size_t data_offset = 2 + bool_size * 2;
    for (size_t i = 0; i < byte_size; i++) {
        if (data_offset + i < size) {
            byte_seq1.data[i] = data[data_offset + i];
        } else {
            byte_seq1.data[i] = 0;
        }
    }

    // 6. Initialize output byte sequence for copy
    byte_seq_copy.data = (uint8_t*)malloc(byte_size * sizeof(uint8_t));
    if (byte_seq_copy.data == NULL) {
        goto cleanup;
    }
    byte_seq_copy.size = byte_size;
    byte_seq_copy.capacity = byte_size;

    // Create proper octet sequence structures for the copy function
    rosidl_runtime_c__octet__Sequence octet_input = {byte_seq1.data, byte_size, byte_size};
    rosidl_runtime_c__octet__Sequence octet_output = {byte_seq_copy.data, byte_size, byte_size};

    // 7. Copy byte sequence
    if (!rosidl_runtime_c__byte__Sequence__copy(&octet_input, &octet_output)) {
        goto cleanup;
    }

    // Verify byte copy by comparing contents
    bool byte_copy_valid = true;
    for (size_t i = 0; i < byte_size; i++) {
        if (byte_seq1.data[i] != byte_seq_copy.data[i]) {
            byte_copy_valid = false;
            break;
        }
    }
    (void)byte_copy_valid;

cleanup:
    // 8. Finalize all sequences (free memory)
    rosidl_runtime_c__bool__Sequence__fini(&bool_seq1);
    rosidl_runtime_c__bool__Sequence__fini(&bool_seq2);
    rosidl_runtime_c__bool__Sequence__fini(&bool_seq_copy);

    // Free manually allocated byte sequences
    if (byte_seq1.data != NULL) {
        free(byte_seq1.data);
    }
    if (byte_seq_copy.data != NULL) {
        free(byte_seq_copy.data);
    }

    return 0;
}

// Implement the API functions as provided in source code
bool rosidl_runtime_c__bool__Sequence__are_equal(
    const rosidl_runtime_c__boolean__Sequence * lhs,
    const rosidl_runtime_c__boolean__Sequence * rhs)
{
    if (lhs == NULL || rhs == NULL) {
        return lhs == rhs;
    }
    if (lhs->size != rhs->size) {
        return false;
    }
    for (size_t i = 0; i < lhs->size; i++) {
        if (lhs->data[i] != rhs->data[i]) {
            return false;
        }
    }
    return true;
}

bool rosidl_runtime_c__bool__Sequence__init(
    rosidl_runtime_c__boolean__Sequence * sequence, size_t size)
{
    if (sequence == NULL) {
        return false;
    }
    
    sequence->data = (bool*)calloc(size, sizeof(bool));
    if (sequence->data == NULL && size > 0) {
        return false;
    }
    
    sequence->size = size;
    sequence->capacity = size;
    return true;
}

bool rosidl_runtime_c__byte__Sequence__copy(
    const rosidl_runtime_c__octet__Sequence * input,
    rosidl_runtime_c__octet__Sequence * output)
{
    if (input == NULL || output == NULL) {
        return false;
    }
    if (output->capacity < input->size) {
        return false;
    }
    
    memcpy(output->data, input->data, input->size * sizeof(uint8_t));
    output->size = input->size;
    return true;
}

bool rosidl_runtime_c__bool__Sequence__copy(
    const rosidl_runtime_c__boolean__Sequence * input,
    rosidl_runtime_c__boolean__Sequence * output)
{
    if (input == NULL || output == NULL) {
        return false;
    }
    if (output->capacity < input->size) {
        return false;
    }
    
    memcpy(output->data, input->data, input->size * sizeof(bool));
    output->size = input->size;
    return true;
}

void rosidl_runtime_c__bool__Sequence__fini(
    rosidl_runtime_c__boolean__Sequence * sequence)
{
    if (sequence != NULL) {
        if (sequence->data != NULL) {
            free(sequence->data);
            sequence->data = NULL;
        }
        sequence->size = 0;
        sequence->capacity = 0;
    }
}
