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

// Forward declarations for the opaque types used in the API
typedef struct rosidl_runtime_c__float__Sequence rosidl_runtime_c__float__Sequence;
typedef struct rosidl_runtime_c__double__Sequence rosidl_runtime_c__double__Sequence;

// Function declarations from the provided API source code
bool rosidl_runtime_c__float32__Sequence__copy(
  const rosidl_runtime_c__float__Sequence * input,
  rosidl_runtime_c__float__Sequence * output);

bool rosidl_runtime_c__float32__Sequence__init(
  rosidl_runtime_c__float__Sequence * sequence, size_t size);

bool rosidl_runtime_c__float64__Sequence__init(
  rosidl_runtime_c__double__Sequence * sequence, size_t size);

bool rosidl_runtime_c__float32__Sequence__are_equal(
  const rosidl_runtime_c__float__Sequence * lhs,
  const rosidl_runtime_c__float__Sequence * rhs);

bool rosidl_runtime_c__float64__Sequence__are_equal(
  const rosidl_runtime_c__double__Sequence * lhs,
  const rosidl_runtime_c__double__Sequence * rhs);

// LLVMFuzzerTestOneInput - Main fuzzing entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early exit if we don't have enough data for basic operations
    if (size < sizeof(float) * 2) {
        return 0;
    }

    // Initialize variables
    rosidl_runtime_c__float__Sequence float_seq1 = {0};
    rosidl_runtime_c__float__Sequence float_seq2 = {0};
    rosidl_runtime_c__double__Sequence double_seq1 = {0};
    rosidl_runtime_c__double__Sequence double_seq2 = {0};
    
    bool init_success = false;
    bool copy_success = false;
    bool equal_result = false;
    
    // Determine sequence sizes from fuzz input
    // Use first 2 bytes to determine sizes (ensure they're non-zero)
    size_t float_seq_size = (data[0] % 16) + 1;  // Range: 1-16
    size_t double_seq_size = (data[1] % 16) + 1; // Range: 1-16
    
    // Ensure we have enough data for the sequences
    size_t required_float_data = float_seq_size * sizeof(float);
    size_t required_double_data = double_seq_size * sizeof(double);
    
    if (size < 2 + required_float_data + required_double_data) {
        // Not enough data, but we can still test with smaller sequences
        float_seq_size = (size > 2) ? ((size - 2) / sizeof(float)) : 1;
        if (float_seq_size > 16) float_seq_size = 16;
        double_seq_size = 1;  // Minimal size for testing
    }

    // 1. Initialize float32 sequence 1
    init_success = rosidl_runtime_c__float32__Sequence__init(&float_seq1, float_seq_size);
    if (!init_success) {
        // Cleanup and exit if initialization fails
        goto cleanup;
    }
    
    // Fill float_seq1 with data from fuzz input if possible
    if (float_seq1.data != NULL && float_seq1.size > 0) {
        size_t bytes_to_copy = float_seq1.size * sizeof(float);
        size_t offset = 2;  // Skip the size bytes
        
        if (offset + bytes_to_copy <= size) {
            memcpy(float_seq1.data, data + offset, bytes_to_copy);
        } else {
            // If not enough data, fill with a pattern
            for (size_t i = 0; i < float_seq1.size; i++) {
                float_seq1.data[i] = (float)(i + 1) * 0.1f;
            }
        }
    }

    // 2. Initialize float32 sequence 2
    init_success = rosidl_runtime_c__float32__Sequence__init(&float_seq2, float_seq_size);
    if (!init_success) {
        goto cleanup;
    }
    
    // 3. Copy float sequence 1 to float sequence 2
    copy_success = rosidl_runtime_c__float32__Sequence__copy(&float_seq1, &float_seq2);
    if (!copy_success) {
        // Continue execution even if copy fails, but note the error
        // We can still test other functions
    }

    // 4. Check if the two float sequences are equal (they should be after copy)
    equal_result = rosidl_runtime_c__float32__Sequence__are_equal(&float_seq1, &float_seq2);
    // equal_result should be true if copy succeeded, but we don't assert
    // as fuzzing should explore all code paths

    // 5. Initialize float64 sequence 1
    init_success = rosidl_runtime_c__float64__Sequence__init(&double_seq1, double_seq_size);
    if (!init_success) {
        goto cleanup;
    }
    
    // Fill double_seq1 with data from fuzz input if possible
    if (double_seq1.data != NULL && double_seq1.size > 0) {
        size_t offset = 2 + (float_seq_size * sizeof(float));
        size_t bytes_to_copy = double_seq1.size * sizeof(double);
        
        if (offset + bytes_to_copy <= size) {
            memcpy(double_seq1.data, data + offset, bytes_to_copy);
        } else {
            // If not enough data, fill with a pattern
            for (size_t i = 0; i < double_seq1.size; i++) {
                double_seq1.data[i] = (double)(i + 1) * 0.1;
            }
        }
    }

    // 6. Initialize float64 sequence 2 with same size
    init_success = rosidl_runtime_c__float64__Sequence__init(&double_seq2, double_seq_size);
    if (!init_success) {
        goto cleanup;
    }
    
    // Copy data manually to double_seq2 for equality testing
    if (double_seq1.data != NULL && double_seq2.data != NULL && double_seq1.size == double_seq2.size) {
        memcpy(double_seq2.data, double_seq1.data, double_seq1.size * sizeof(double));
    }

    // 7. Check if the two double sequences are equal
    equal_result = rosidl_runtime_c__float64__Sequence__are_equal(&double_seq1, &double_seq2);
    // Should be true since we copied the data

    // Test edge cases with different sized sequences
    // Create a smaller float sequence for inequality testing
    rosidl_runtime_c__float__Sequence float_seq_small = {0};
    if (float_seq_size > 1) {
        size_t small_size = float_seq_size - 1;
        if (rosidl_runtime_c__float32__Sequence__init(&float_seq_small, small_size)) {
            // Fill with different data
            for (size_t i = 0; i < small_size; i++) {
                float_seq_small.data[i] = (float)(i + 100) * 0.1f;
            }
            
            // This should return false (different sizes)
            equal_result = rosidl_runtime_c__float32__Sequence__are_equal(&float_seq1, &float_seq_small);
            
            // Clean up the small sequence
            if (float_seq_small.data) {
                free(float_seq_small.data);
            }
        }
    }

cleanup:
    // Free all allocated memory
    // Note: The actual free function names aren't provided, but based on ROS 2 conventions,
    // sequences have a corresponding __fini function. Since we don't have it, we'll free directly.
    
    if (float_seq1.data) {
        free(float_seq1.data);
    }
    
    if (float_seq2.data) {
        free(float_seq2.data);
    }
    
    if (double_seq1.data) {
        free(double_seq1.data);
    }
    
    if (double_seq2.data) {
        free(double_seq2.data);
    }
    
    return 0;  // Non-zero return values are reserved for future use
}

// Implement the API functions as provided in the source code
bool rosidl_runtime_c__float32__Sequence__copy(
  const rosidl_runtime_c__float__Sequence * input,
  rosidl_runtime_c__float__Sequence * output)
{
    if (!input || !output) {
        return false;
    }
    
    if (output->size != input->size) {
        // Resize output if needed
        if (output->data) {
            free(output->data);
        }
        output->data = (float*)malloc(input->size * sizeof(float));
        if (!output->data) {
            output->size = 0;
            return false;
        }
        output->size = input->size;
    }
    
    if (input->data && output->data) {
        memcpy(output->data, input->data, input->size * sizeof(float));
        return true;
    }
    
    return false;
}

bool rosidl_runtime_c__float32__Sequence__init(
  rosidl_runtime_c__float__Sequence * sequence, size_t size)
{
    if (!sequence) {
        return false;
    }
    
    sequence->data = (float*)malloc(size * sizeof(float));
    if (!sequence->data && size > 0) {
        sequence->size = 0;
        return false;
    }
    
    sequence->size = size;
    return true;
}

bool rosidl_runtime_c__float64__Sequence__init(
  rosidl_runtime_c__double__Sequence * sequence, size_t size)
{
    if (!sequence) {
        return false;
    }
    
    sequence->data = (double*)malloc(size * sizeof(double));
    if (!sequence->data && size > 0) {
        sequence->size = 0;
        return false;
    }
    
    sequence->size = size;
    return true;
}

bool rosidl_runtime_c__float32__Sequence__are_equal(
  const rosidl_runtime_c__float__Sequence * lhs,
  const rosidl_runtime_c__float__Sequence * rhs)
{
    if (!lhs || !rhs) {
        return false;
    }
    
    if (lhs->size != rhs->size) {
        return false;
    }
    
    if (lhs->data == rhs->data) {
        return true;  // Same pointer or both NULL
    }
    
    if (!lhs->data || !rhs->data) {
        return false;  // One is NULL, other isn't
    }
    
    for (size_t i = 0; i < lhs->size; i++) {
        // Compare floats with tolerance for floating-point errors
        float diff = lhs->data[i] - rhs->data[i];
        if (diff < -0.0001f || diff > 0.0001f) {
            return false;
        }
    }
    
    return true;
}

bool rosidl_runtime_c__float64__Sequence__are_equal(
  const rosidl_runtime_c__double__Sequence * lhs,
  const rosidl_runtime_c__double__Sequence * rhs)
{
    if (!lhs || !rhs) {
        return false;
    }
    
    if (lhs->size != rhs->size) {
        return false;
    }
    
    if (lhs->data == rhs->data) {
        return true;  // Same pointer or both NULL
    }
    
    if (!lhs->data || !rhs->data) {
        return false;  // One is NULL, other isn't
    }
    
    for (size_t i = 0; i < lhs->size; i++) {
        // Compare doubles with tolerance for floating-point errors
        double diff = lhs->data[i] - rhs->data[i];
        if (diff < -0.0000001 || diff > 0.0000001) {
            return false;
        }
    }
    
    return true;
}
