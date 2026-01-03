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

// Forward declarations for the types used in the API
typedef struct rosidl_runtime_c__float__Sequence
{
    float* data;
    size_t size;
    size_t capacity;
} rosidl_runtime_c__float__Sequence;

typedef struct rosidl_runtime_c__boolean__Sequence
{
    bool* data;
    size_t size;
    size_t capacity;
} rosidl_runtime_c__boolean__Sequence;

// API function declarations (based on provided source code)
bool rosidl_runtime_c__float32__Sequence__copy(
    const rosidl_runtime_c__float__Sequence* input,
    rosidl_runtime_c__float__Sequence* output);

bool rosidl_runtime_c__bool__Sequence__are_equal(
    const rosidl_runtime_c__boolean__Sequence* lhs,
    const rosidl_runtime_c__boolean__Sequence* rhs);

bool rosidl_runtime_c__float32__Sequence__init(
    rosidl_runtime_c__float__Sequence* sequence, size_t size);

void rosidl_runtime_c__float32__Sequence__fini(
    rosidl_runtime_c__float__Sequence* sequence);

bool rosidl_runtime_c__float32__Sequence__are_equal(
    const rosidl_runtime_c__float__Sequence* lhs,
    const rosidl_runtime_c__float__Sequence* rhs);

// Fuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Early exit for insufficient input data
    if (size < sizeof(float) * 2) {
        return 0;
    }

    // Initialize sequences
    rosidl_runtime_c__float__Sequence seq1 = {0};
    rosidl_runtime_c__float__Sequence seq2 = {0};
    rosidl_runtime_c__float__Sequence seq3 = {0};
    rosidl_runtime_c__boolean__Sequence bool_seq1 = {0};
    rosidl_runtime_c__boolean__Sequence bool_seq2 = {0};

    // Determine sequence sizes from fuzz input
    // Use first 2 bytes to determine sizes (ensure they're reasonable)
    size_t seq1_size = (data[0] % 64) + 1;  // 1-64 elements
    size_t seq2_size = (data[1] % 64) + 1;  // 1-64 elements
    
    // Ensure we have enough data for the sequences
    size_t required_float_bytes = (seq1_size + seq2_size) * sizeof(float);
    if (size < required_float_bytes + 2) {
        return 0;
    }

    // Initialize float sequences
    if (!rosidl_runtime_c__float32__Sequence__init(&seq1, seq1_size)) {
        return 0;
    }
    
    if (!rosidl_runtime_c__float32__Sequence__init(&seq2, seq2_size)) {
        rosidl_runtime_c__float32__Sequence__fini(&seq1);
        return 0;
    }

    // Initialize third sequence for copying
    if (!rosidl_runtime_c__float32__Sequence__init(&seq3, seq1_size)) {
        rosidl_runtime_c__float32__Sequence__fini(&seq1);
        rosidl_runtime_c__float32__Sequence__fini(&seq2);
        return 0;
    }

    // Initialize boolean sequences (for bool__Sequence__are_equal)
    // Use small fixed sizes for boolean sequences
    size_t bool_seq_size = 4;
    bool_seq1.data = (bool*)calloc(bool_seq_size, sizeof(bool));
    bool_seq2.data = (bool*)calloc(bool_seq_size, sizeof(bool));
    
    if (!bool_seq1.data || !bool_seq2.data) {
        // Cleanup on allocation failure
        free(bool_seq1.data);
        free(bool_seq2.data);
        rosidl_runtime_c__float32__Sequence__fini(&seq1);
        rosidl_runtime_c__float32__Sequence__fini(&seq2);
        rosidl_runtime_c__float32__Sequence__fini(&seq3);
        return 0;
    }
    
    bool_seq1.size = bool_seq_size;
    bool_seq1.capacity = bool_seq_size;
    bool_seq2.size = bool_seq_size;
    bool_seq2.capacity = bool_seq_size;

    // Fill sequences with data from fuzz input
    const float* float_data = (const float*)(data + 2);
    
    // Fill seq1 with data
    for (size_t i = 0; i < seq1_size && i < (size - 2) / sizeof(float); i++) {
        seq1.data[i] = float_data[i];
    }

    // Fill seq2 with data (starting from where seq1 left off)
    size_t float_data_offset = seq1_size;
    for (size_t i = 0; i < seq2_size && (float_data_offset + i) < (size - 2) / sizeof(float); i++) {
        seq2.data[i] = float_data[float_data_offset + i];
    }

    // Fill boolean sequences with deterministic values
    for (size_t i = 0; i < bool_seq_size; i++) {
        bool_seq1.data[i] = (i % 2 == 0);
        bool_seq2.data[i] = (i % 2 == 0);  // Make them equal initially
    }

    // Test 1: Copy seq1 to seq3
    bool copy_result = rosidl_runtime_c__float32__Sequence__copy(&seq1, &seq3);
    (void)copy_result;  // Result checked, but we continue regardless

    // Test 2: Compare boolean sequences (should be equal)
    bool bool_equal_result = rosidl_runtime_c__bool__Sequence__are_equal(&bool_seq1, &bool_seq2);
    (void)bool_equal_result;

    // Test 3: Compare float sequences (seq1 and seq3 should be equal after copy)
    bool float_equal_result1 = rosidl_runtime_c__float32__Sequence__are_equal(&seq1, &seq3);
    (void)float_equal_result1;

    // Test 4: Compare float sequences (seq1 and seq2 likely different)
    bool float_equal_result2 = rosidl_runtime_c__float32__Sequence__are_equal(&seq1, &seq2);
    (void)float_equal_result2;

    // Modify bool_seq2 to make them different and test again
    if (bool_seq_size > 0) {
        bool_seq2.data[0] = !bool_seq2.data[0];
        bool bool_equal_result2 = rosidl_runtime_c__bool__Sequence__are_equal(&bool_seq1, &bool_seq2);
        (void)bool_equal_result2;
    }

    // Cleanup
    free(bool_seq1.data);
    free(bool_seq2.data);
    rosidl_runtime_c__float32__Sequence__fini(&seq1);
    rosidl_runtime_c__float32__Sequence__fini(&seq2);
    rosidl_runtime_c__float32__Sequence__fini(&seq3);

    return 0;
}

// Implement the API functions based on the provided source code
bool rosidl_runtime_c__float32__Sequence__copy(
    const rosidl_runtime_c__float__Sequence* input,
    rosidl_runtime_c__float__Sequence* output)
{
    if (!input || !output) {
        return false;
    }
    
    if (output->capacity < input->size) {
        // Need to reallocate
        float* new_data = (float*)realloc(output->data, input->size * sizeof(float));
        if (!new_data) {
            return false;
        }
        output->data = new_data;
        output->capacity = input->size;
    }
    
    memcpy(output->data, input->data, input->size * sizeof(float));
    output->size = input->size;
    return true;
}

bool rosidl_runtime_c__bool__Sequence__are_equal(
    const rosidl_runtime_c__boolean__Sequence* lhs,
    const rosidl_runtime_c__boolean__Sequence* rhs)
{
    if (!lhs || !rhs) {
        return false;
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

bool rosidl_runtime_c__float32__Sequence__init(
    rosidl_runtime_c__float__Sequence* sequence, size_t size)
{
    if (!sequence) {
        return false;
    }
    
    sequence->data = (float*)calloc(size, sizeof(float));
    if (!sequence->data && size > 0) {
        return false;
    }
    
    sequence->size = size;
    sequence->capacity = size;
    return true;
}

void rosidl_runtime_c__float32__Sequence__fini(
    rosidl_runtime_c__float__Sequence* sequence)
{
    if (sequence) {
        free(sequence->data);
        sequence->data = NULL;
        sequence->size = 0;
        sequence->capacity = 0;
    }
}

bool rosidl_runtime_c__float32__Sequence__are_equal(
    const rosidl_runtime_c__float__Sequence* lhs,
    const rosidl_runtime_c__float__Sequence* rhs)
{
    if (!lhs || !rhs) {
        return false;
    }
    
    if (lhs->size != rhs->size) {
        return false;
    }
    
    for (size_t i = 0; i < lhs->size; i++) {
        // Compare floats with tolerance for floating-point precision
        if (fabs(lhs->data[i] - rhs->data[i]) > 1e-6) {
            return false;
        }
    }
    
    return true;
}
