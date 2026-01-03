#include <rosidl_runtime_c/message_type_support.h>
#include <rosidl_runtime_c/primitives_sequence_functions.h>
#include <rosidl_runtime_c/sequence_bound.h>
#include <rosidl_runtime_c/service_type_support.h>
#include <rosidl_runtime_c/string_functions.h>
#include <rosidl_runtime_c/u16string_functions.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <rcutils/allocator.h>

// LLVMFuzzerTestOneInput function for fuzzing ROS 2 string sequence operations
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early return if no data or insufficient size for basic operations
    if (!data || size < 2) {
        return 0;
    }

    // Initialize default allocator
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    
    // Use first byte to determine sequence size (limit to reasonable value)
    size_t seq_size = (data[0] % 16) + 1;  // 1-16 elements
    
    // Create and initialize source sequence
    rosidl_runtime_c__String__Sequence src_sequence;
    if (!rosidl_runtime_c__String__Sequence__init(&src_sequence, seq_size)) {
        // Initialization failed, nothing to clean up
        return 0;
    }

    // Fill source sequence with data from fuzz input
    size_t data_offset = 1;  // Skip first byte used for seq_size
    for (size_t i = 0; i < seq_size && i < src_sequence.capacity; i++) {
        if (data_offset >= size) {
            // Wrap around if we run out of fuzz data
            data_offset = 1;
        }
        
        // Calculate string length for this element (1-255 bytes)
        size_t str_len = (data[data_offset] % 255) + 1;
        data_offset = (data_offset + 1) % size;
        
        // Ensure we don't exceed available fuzz data
        if (str_len > size - data_offset) {
            str_len = size - data_offset;
            if (str_len == 0) {
                // No data left, use empty string
                continue;
            }
        }
        
        // Reallocate string buffer to hold the data
        char *new_data = (char *)allocator.reallocate(
            src_sequence.data[i].data, 
            str_len + 1,  // +1 for null terminator
            allocator.state
        );
        
        if (new_data) {
            src_sequence.data[i].data = new_data;
            // Copy fuzz data into string
            memcpy(src_sequence.data[i].data, data + data_offset, str_len);
            src_sequence.data[i].data[str_len] = '\0';
            src_sequence.data[i].size = str_len;
            src_sequence.data[i].capacity = str_len + 1;
            
            data_offset = (data_offset + str_len) % size;
        }
        // If reallocation fails, keep the initialized empty string
    }

    // Create destination sequence with different initial size
    rosidl_runtime_c__String__Sequence dst_sequence;
    size_t dst_init_size = (seq_size > 1) ? (seq_size / 2) : 1;
    
    if (!rosidl_runtime_c__String__Sequence__init(&dst_sequence, dst_init_size)) {
        // Clean up source sequence before returning
        rosidl_runtime_c__String__Sequence__fini(&src_sequence);
        return 0;
    }

    // Test sequence copy operation
    bool copy_success = rosidl_runtime_c__String__Sequence__copy(&src_sequence, &dst_sequence);
    
    if (copy_success) {
        // Verify copy was successful by checking sizes match
        assert(src_sequence.size == dst_sequence.size);
        
        // Verify each string was copied (optional, for completeness)
        for (size_t i = 0; i < src_sequence.size && i < dst_sequence.size; i++) {
            if (src_sequence.data[i].data && dst_sequence.data[i].data) {
                assert(strcmp(src_sequence.data[i].data, dst_sequence.data[i].data) == 0);
            }
        }
    }

    // Test individual string initialization and finalization
    rosidl_runtime_c__String single_string;
    if (rosidl_runtime_c__String__init(&single_string)) {
        // Try to populate with some fuzz data if available
        if (size > 1) {
            size_t str_len = (data[0] % 64) + 1;  // 1-64 bytes
            if (str_len > size - 1) {
                str_len = size - 1;
            }
            
            char *new_data = (char *)allocator.reallocate(
                single_string.data,
                str_len + 1,
                allocator.state
            );
            
            if (new_data) {
                single_string.data = new_data;
                memcpy(single_string.data, data + 1, str_len);
                single_string.data[str_len] = '\0';
                single_string.size = str_len;
                single_string.capacity = str_len + 1;
            }
        }
        
        // Clean up the single string
        rosidl_runtime_c__String__fini(&single_string);
    }

    // Clean up all allocated resources
    rosidl_runtime_c__String__Sequence__fini(&dst_sequence);
    rosidl_runtime_c__String__Sequence__fini(&src_sequence);

    return 0;
}
