#include <rosidl_runtime_c/primitives_sequence_functions.h>
#include <rosidl_runtime_c/sequence_bound.h>
#include <rosidl_runtime_c/string_functions.h>
#include <rosidl_runtime_c/u16string_functions.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <rcutils/allocator.h>

// Function prototype for boolean sequence equality (not in provided headers but needed)
// Based on API summary, we need to include the proper header
#include <rosidl_runtime_c/primitives_sequence_functions.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early return if no data
    if (data == NULL || size == 0) {
        return 0;
    }

    // Initialize variables
    rosidl_runtime_c__String str1 = {0};
    rosidl_runtime_c__String str2 = {0};
    rosidl_runtime_c__String__Sequence str_sequence1 = {0};
    rosidl_runtime_c__String__Sequence str_sequence2 = {0};
    rosidl_runtime_c__boolean__Sequence bool_sequence1 = {0};
    rosidl_runtime_c__boolean__Sequence bool_sequence2 = {0};
    
    bool success = false;
    size_t sequence_size = 0;
    size_t bool_sequence_size = 0;
    
    // Use fuzz input to determine sequence sizes (bounded to prevent excessive allocation)
    // Use modulo to get reasonable sizes (max 16 elements to avoid OOM)
    sequence_size = (size_t)(data[0] % 16) + 1;
    bool_sequence_size = (size_t)(data[size > 1 ? 1 : 0] % 16) + 1;
    
    // 1. Initialize individual strings
    success = rosidl_runtime_c__String__init(&str1);
    if (!success) {
        // Initialization failed, clean up and return
        goto cleanup;
    }
    
    success = rosidl_runtime_c__String__init(&str2);
    if (!success) {
        // Initialization failed, clean up and return
        goto cleanup;
    }
    
    // 2. Initialize string sequences
    success = rosidl_runtime_c__String__Sequence__init(&str_sequence1, sequence_size);
    if (!success) {
        goto cleanup;
    }
    
    // Initialize second sequence with potentially different size
    size_t sequence_size2 = (size_t)(data[size > 2 ? 2 : 0] % 16) + 1;
    success = rosidl_runtime_c__String__Sequence__init(&str_sequence2, sequence_size2);
    if (!success) {
        goto cleanup;
    }
    
    // 3. Initialize boolean sequences for comparison
    // Note: We need to use the proper initialization function for boolean sequences
    // Since it's not in the provided API list, we'll allocate manually but carefully
    if (bool_sequence_size > 0) {
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        bool *bool_data = (bool *)allocator.zero_allocate(
            bool_sequence_size, sizeof(bool), allocator.state);
        if (!bool_data) {
            goto cleanup;
        }
        bool_sequence1.data = bool_data;
        bool_sequence1.size = bool_sequence_size;
        bool_sequence1.capacity = bool_sequence_size;
        
        // Initialize second boolean sequence
        size_t bool_sequence_size2 = (size_t)(data[size > 3 ? 3 : 0] % 16) + 1;
        bool *bool_data2 = (bool *)allocator.zero_allocate(
            bool_sequence_size2, sizeof(bool), allocator.state);
        if (!bool_data2) {
            allocator.deallocate(bool_data, allocator.state);
            goto cleanup;
        }
        bool_sequence2.data = bool_data2;
        bool_sequence2.size = bool_sequence_size2;
        bool_sequence2.capacity = bool_sequence_size2;
        
        // Fill boolean sequences with data from fuzz input (safely)
        for (size_t i = 0; i < bool_sequence1.size && i + 4 < size; i++) {
            bool_sequence1.data[i] = (data[i + 4] & 0x01) != 0;
        }
        
        for (size_t i = 0; i < bool_sequence2.size && i + 4 < size; i++) {
            bool_sequence2.data[i] = (data[i + 4] & 0x02) != 0;
        }
        
        // 4. Call boolean sequence comparison function
        // Note: The provided function rosidl_runtime_c__bool__Sequence__are_equal
        // appears to be a wrapper. We'll use the actual function from the primitives header.
        // However, since we don't have the exact function signature in provided headers,
        // we'll use a safe approach.
        bool sequences_equal = false;
        
        // Check if we can compare (both sequences initialized)
        if (bool_sequence1.data && bool_sequence2.data) {
            // Use the actual comparison function from the proper header
            // This matches the API summary description
            sequences_equal = rosidl_runtime_c__boolean__Sequence__are_equal(
                &bool_sequence1, &bool_sequence2);
            
            // The result is not used, but the function is called as required
            (void)sequences_equal;
        }
    }
    
    // 5. Clean up string sequences (call fini functions)
    rosidl_runtime_c__String__Sequence__fini(&str_sequence1);
    rosidl_runtime_c__String__Sequence__fini(&str_sequence2);
    
    // 6. Clean up individual strings
    rosidl_runtime_c__String__fini(&str1);
    rosidl_runtime_c__String__fini(&str2);
    
    // Clean up boolean sequences if they were allocated
    if (bool_sequence1.data) {
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        allocator.deallocate(bool_sequence1.data, allocator.state);
        bool_sequence1.data = NULL;
        bool_sequence1.size = 0;
        bool_sequence1.capacity = 0;
    }
    
    if (bool_sequence2.data) {
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        allocator.deallocate(bool_sequence2.data, allocator.state);
        bool_sequence2.data = NULL;
        bool_sequence2.size = 0;
        bool_sequence2.capacity = 0;
    }
    
    return 0;

cleanup:
    // Clean up any allocated resources in case of error
    if (str_sequence1.data) {
        rosidl_runtime_c__String__Sequence__fini(&str_sequence1);
    }
    
    if (str_sequence2.data) {
        rosidl_runtime_c__String__Sequence__fini(&str_sequence2);
    }
    
    if (str1.data) {
        rosidl_runtime_c__String__fini(&str1);
    }
    
    if (str2.data) {
        rosidl_runtime_c__String__fini(&str2);
    }
    
    if (bool_sequence1.data) {
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        allocator.deallocate(bool_sequence1.data, allocator.state);
    }
    
    if (bool_sequence2.data) {
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        allocator.deallocate(bool_sequence2.data, allocator.state);
    }
    
    return 0;
}
