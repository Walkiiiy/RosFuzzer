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
#include <assert.h>

// Fuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early return if insufficient data
    if (size < 4) {
        return 0;
    }

    // Initialize all variables
    rosidl_runtime_c__float__Sequence float32_seq_input = {0};
    rosidl_runtime_c__float__Sequence float32_seq_output = {0};
    rosidl_runtime_c__octet__Sequence byte_seq = {0};
    rosidl_runtime_c__U16String u16str = {0};
    rosidl_runtime_c__double__Sequence float64_seq_lhs = {0};
    rosidl_runtime_c__double__Sequence float64_seq_rhs = {0};
    
    // Variables for sequence bounds
    rosidl_runtime_c__Sequence__bound bound_handle = {0};
    const rosidl_runtime_c__Sequence__bound *retrieved_handle = NULL;
    
    bool success = false;
    int cleanup_needed = 0; // Track what needs cleanup

    // 1. Initialize float32 sequence from fuzz data
    // Use first 4 bytes to determine sequence size (modulo to limit size)
    size_t float32_seq_size = (data[0] % 16) + 1; // Size between 1-16
    if (float32_seq_size * sizeof(float) > size - 1) {
        // Not enough data, adjust size
        float32_seq_size = (size - 1) / sizeof(float);
        if (float32_seq_size == 0) {
            float32_seq_size = 1;
        }
    }

    // Allocate memory for float32 sequence
    float32_seq_input.data = (float*)malloc(float32_seq_size * sizeof(float));
    float32_seq_input.size = float32_seq_size;
    float32_seq_input.capacity = float32_seq_size;
    
    if (float32_seq_input.data == NULL) {
        goto cleanup;
    }
    cleanup_needed |= 0x01; // Mark float32_seq_input for cleanup

    // Fill with fuzz data (convert bytes to floats)
    for (size_t i = 0; i < float32_seq_size && (i + 1) * sizeof(float) <= size; i++) {
        uint32_t temp = 0;
        size_t offset = i * sizeof(float);
        for (size_t j = 0; j < sizeof(float) && (offset + j) < size; j++) {
            temp |= (uint32_t)data[offset + j] << (8 * j);
        }
        float32_seq_input.data[i] = *(float*)&temp;
    }

    // 2. Copy float32 sequence using rosidl_runtime_c__float32__Sequence__copy
    // First allocate output sequence
    float32_seq_output.data = (float*)malloc(float32_seq_size * sizeof(float));
    float32_seq_output.size = float32_seq_size;
    float32_seq_output.capacity = float32_seq_size;
    
    if (float32_seq_output.data == NULL) {
        goto cleanup;
    }
    cleanup_needed |= 0x02; // Mark float32_seq_output for cleanup

    success = rosidl_runtime_c__float32__Sequence__copy(&float32_seq_input, &float32_seq_output);
    if (!success) {
        // Copy failed, but continue with other APIs
    }

    // 3. Initialize byte sequence for rosidl_runtime_c__byte__Sequence__fini
    size_t byte_seq_size = (data[1] % 32) + 1; // Size between 1-32
    if (byte_seq_size > size - 2) {
        byte_seq_size = size - 2;
        if (byte_seq_size == 0) {
            byte_seq_size = 1;
        }
    }

    byte_seq.data = (unsigned char*)malloc(byte_seq_size);
    byte_seq.size = byte_seq_size;
    byte_seq.capacity = byte_seq_size;
    
    if (byte_seq.data == NULL) {
        goto cleanup;
    }
    cleanup_needed |= 0x04; // Mark byte_seq for cleanup

    // Fill byte sequence with fuzz data
    size_t data_offset = 2;
    for (size_t i = 0; i < byte_seq_size && data_offset + i < size; i++) {
        byte_seq.data[i] = data[data_offset + i];
    }

    // 4. Initialize UTF-16 string for rosidl_runtime_c__U16String__assignn_from_char
    // Ensure we have enough data and n is even
    size_t char_data_size = (data[2] % 64) + 1; // Size between 1-64
    if (char_data_size > size - 3) {
        char_data_size = size - 3;
    }
    
    // Make sure n is even for UTF-16 encoding
    if (char_data_size % 2 != 0 && char_data_size > 0) {
        char_data_size--;
    }
    if (char_data_size == 0 && size >= 4) {
        char_data_size = 2; // Minimum even size
    }

    // Initialize U16String
    if (!rosidl_runtime_c__U16String__init(&u16str, 0)) {
        goto cleanup;
    }
    cleanup_needed |= 0x08; // Mark u16str for cleanup

    // Prepare char data for assignment
    const char* char_data = (const char*)(data + 3);
    size_t actual_char_size = char_data_size;
    if (3 + char_data_size > size) {
        actual_char_size = size - 3;
        // Ensure even
        if (actual_char_size % 2 != 0 && actual_char_size > 0) {
            actual_char_size--;
        }
    }

    if (actual_char_size > 0) {
        success = rosidl_runtime_c__U16String__assignn_from_char(&u16str, char_data, actual_char_size);
        // Don't check success - continue even if assignment fails
    }

    // 5. Initialize float64 sequences for rosidl_runtime_c__float64__Sequence__are_equal
    size_t float64_seq_size = (data[3] % 8) + 1; // Size between 1-8
    if (float64_seq_size * sizeof(double) > size - 4) {
        float64_seq_size = (size - 4) / sizeof(double);
        if (float64_seq_size == 0) {
            float64_seq_size = 1;
        }
    }

    // Allocate and initialize first sequence
    float64_seq_lhs.data = (double*)malloc(float64_seq_size * sizeof(double));
    float64_seq_lhs.size = float64_seq_size;
    float64_seq_lhs.capacity = float64_seq_size;
    
    if (float64_seq_lhs.data == NULL) {
        goto cleanup;
    }
    cleanup_needed |= 0x10; // Mark float64_seq_lhs for cleanup

    // Fill with fuzz data
    for (size_t i = 0; i < float64_seq_size; i++) {
        size_t offset = 4 + i * sizeof(double);
        uint64_t temp = 0;
        for (size_t j = 0; j < sizeof(double) && offset + j < size; j++) {
            temp |= (uint64_t)data[offset + j] << (8 * j);
        }
        float64_seq_lhs.data[i] = *(double*)&temp;
    }

    // Allocate and initialize second sequence (copy of first for equality test)
    float64_seq_rhs.data = (double*)malloc(float64_seq_size * sizeof(double));
    float64_seq_rhs.size = float64_seq_size;
    float64_seq_rhs.capacity = float64_seq_size;
    
    if (float64_seq_rhs.data == NULL) {
        goto cleanup;
    }
    cleanup_needed |= 0x20; // Mark float64_seq_rhs for cleanup

    // Copy data to ensure equality
    memcpy(float64_seq_rhs.data, float64_seq_lhs.data, float64_seq_size * sizeof(double));

    // 6. Test sequence equality
    success = rosidl_runtime_c__float64__Sequence__are_equal(&float64_seq_lhs, &float64_seq_rhs);
    // Should be true since we copied the data

    // 7. Test get_sequence_bound_handle (simulated since we don't have actual handle function)
    // Create a minimal bound handle structure
    bound_handle.func = NULL; // No actual function for fuzzing
    
    // This would normally fail since func is NULL, but we call it anyway
    // to exercise the code path
    retrieved_handle = get_sequence_bound_handle(&bound_handle, "test_identifier");
    // retrieved_handle will be NULL, which is expected

    // 8. Clean up byte sequence using rosidl_runtime_c__byte__Sequence__fini
    rosidl_runtime_c__byte__Sequence__fini(&byte_seq);
    byte_seq.data = NULL; // Prevent double-free
    cleanup_needed &= ~0x04; // Clear byte_seq cleanup flag

cleanup:
    // Clean up all allocated resources
    if (cleanup_needed & 0x01) { // float32_seq_input
        free(float32_seq_input.data);
    }
    if (cleanup_needed & 0x02) { // float32_seq_output
        free(float32_seq_output.data);
    }
    if (cleanup_needed & 0x04) { // byte_seq
        // Already cleaned by fini, but check if data still exists
        if (byte_seq.data) {
            free(byte_seq.data);
        }
    }
    if (cleanup_needed & 0x08) { // u16str
        rosidl_runtime_c__U16String__fini(&u16str);
    }
    if (cleanup_needed & 0x10) { // float64_seq_lhs
        free(float64_seq_lhs.data);
    }
    if (cleanup_needed & 0x20) { // float64_seq_rhs
        free(float64_seq_rhs.data);
    }

    return 0;
}
