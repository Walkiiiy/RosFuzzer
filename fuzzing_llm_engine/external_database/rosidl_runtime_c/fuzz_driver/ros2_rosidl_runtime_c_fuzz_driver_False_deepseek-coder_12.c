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

// Note: <sstream> is a C++ header, but we're writing C code
// We'll use C string functions instead

// Include rcutils headers as they are required by the API
#include <rcutils/allocator.h>

// LLVMFuzzerTestOneInput function
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early return if no data
    if (data == NULL || size == 0) {
        return 0;
    }

    // Initialize variables
    rosidl_runtime_c__String__Sequence seq1 = {0};
    rosidl_runtime_c__String__Sequence seq2 = {0};
    rosidl_runtime_c__String__Sequence seq3 = {0};
    bool success = false;
    
    // Determine sequence size from fuzz input (bounded to prevent excessive allocation)
    // Use first byte modulo reasonable bound (e.g., 16) to avoid huge allocations
    size_t seq_size = 0;
    if (size >= 1) {
        seq_size = data[0] % 16;  // Limit to 0-15 elements
    }
    
    // 1. Test rosidl_runtime_c__String__Sequence__init
    success = rosidl_runtime_c__String__Sequence__init(&seq1, seq_size);
    if (!success) {
        // Initialization failed, clean up and return
        // Note: seq1 may be partially initialized, but fini handles NULL data
        rosidl_runtime_c__String__Sequence__fini(&seq1);
        return 0;
    }
    
    // 2. Populate the sequence with data from fuzz input
    // We'll distribute the fuzz data among the string elements
    if (seq_size > 0 && size > 1) {
        size_t bytes_per_string = (size - 1) / seq_size;
        size_t remaining_bytes = size - 1;
        const uint8_t *string_data = data + 1;
        
        for (size_t i = 0; i < seq_size; i++) {
            // Calculate how many bytes to use for this string
            size_t str_len = bytes_per_string;
            if (i == seq_size - 1) {
                // Last string gets all remaining bytes
                str_len = remaining_bytes;
            }
            
            if (str_len > 0 && remaining_bytes > 0) {
                // Ensure we don't exceed available data
                if (str_len > remaining_bytes) {
                    str_len = remaining_bytes;
                }
                
                // Allocate and copy string data
                // Note: rosidl_runtime_c__String__copy would be needed to set the string,
                // but we don't have that API. Instead, we'll use the initialized strings
                // as-is. The equality test will work with empty/default strings.
                
                // Update pointers for next iteration
                if (remaining_bytes >= str_len) {
                    remaining_bytes -= str_len;
                    string_data += str_len;
                }
            }
        }
    }
    
    // 3. Test rosidl_runtime_c__String__Sequence__copy
    // First initialize seq2 with different size
    size_t seq2_size = (seq_size > 0) ? seq_size / 2 : 0;
    success = rosidl_runtime_c__String__Sequence__init(&seq2, seq2_size);
    if (!success) {
        rosidl_runtime_c__String__Sequence__fini(&seq1);
        rosidl_runtime_c__String__Sequence__fini(&seq2);
        return 0;
    }
    
    // Now copy seq1 to seq2 (seq2 will be reallocated if needed)
    success = rosidl_runtime_c__String__Sequence__copy(&seq1, &seq2);
    if (!success) {
        // Copy failed, clean up and return
        rosidl_runtime_c__String__Sequence__fini(&seq1);
        rosidl_runtime_c__String__Sequence__fini(&seq2);
        return 0;
    }
    
    // 4. Test rosidl_runtime_c__String__Sequence__are_equal
    // Create a third sequence identical to seq1
    success = rosidl_runtime_c__String__Sequence__init(&seq3, seq_size);
    if (!success) {
        rosidl_runtime_c__String__Sequence__fini(&seq1);
        rosidl_runtime_c__String__Sequence__fini(&seq2);
        rosidl_runtime_c__String__Sequence__fini(&seq3);
        return 0;
    }
    
    // Copy seq1 to seq3
    success = rosidl_runtime_c__String__Sequence__copy(&seq1, &seq3);
    if (!success) {
        rosidl_runtime_c__String__Sequence__fini(&seq1);
        rosidl_runtime_c__String__Sequence__fini(&seq2);
        rosidl_runtime_c__String__Sequence__fini(&seq3);
        return 0;
    }
    
    // Test equality - seq1 and seq3 should be equal
    bool are_equal = rosidl_runtime_c__String__Sequence__are_equal(&seq1, &seq3);
    (void)are_equal;  // Use result to avoid unused variable warning
    
    // Test inequality - seq1 and seq2 might not be equal if seq_size != seq2_size
    // But after copy, they should have the same size and content
    if (seq_size == seq2_size) {
        are_equal = rosidl_runtime_c__String__Sequence__are_equal(&seq1, &seq2);
        (void)are_equal;
    }
    
    // 5. Test rosidl_runtime_c__String__fini on individual strings
    // This is already called internally by the sequence fini functions,
    // but we can demonstrate it on a standalone string if we create one
    rosidl_runtime_c__String standalone_string = {0};
    // Note: We don't have rosidl_runtime_c__String__init in our API list,
    // so we can't properly initialize it. We'll just demonstrate the fini call
    // on an uninitialized string (which is safe according to the implementation).
    rosidl_runtime_c__String__fini(&standalone_string);
    
    // 6. Clean up all sequences
    rosidl_runtime_c__String__Sequence__fini(&seq1);
    rosidl_runtime_c__String__Sequence__fini(&seq2);
    rosidl_runtime_c__String__Sequence__fini(&seq3);
    
    return 0;
}
