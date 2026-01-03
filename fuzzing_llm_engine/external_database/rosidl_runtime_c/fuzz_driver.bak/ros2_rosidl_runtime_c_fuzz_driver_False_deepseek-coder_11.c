#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

// ROS 2 runtime headers
#include "rosidl_runtime_c/message_type_support.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"
#include "rosidl_runtime_c/sequence_bound.h"
#include "rosidl_runtime_c/service_type_support.h"
#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_runtime_c/u16string_functions.h"

// Forward declarations for rcutils types and functions
typedef struct rcutils_allocator_s rcutils_allocator_t;

// Mock rcutils functions since they're not provided in the headers
// These are simplified versions for compilation
static rcutils_allocator_t rcutils_get_default_allocator(void) {
    static rcutils_allocator_t allocator = {
        .allocate = malloc,
        .deallocate = free,
        .reallocate = realloc,
        .zero_allocate = calloc,
        .state = NULL
    };
    return allocator;
}

// Mock RCUTILS_CAN_RETURN_WITH_ERROR_OF macro
#define RCUTILS_CAN_RETURN_WITH_ERROR_OF(x)

// ROS 2 runtime string structure
typedef struct rosidl_runtime_c__String {
    char *data;
    size_t size;
    size_t capacity;
} rosidl_runtime_c__String;

// ROS 2 runtime string sequence structure
typedef struct rosidl_runtime_c__String__Sequence {
    rosidl_runtime_c__String *data;
    size_t size;
    size_t capacity;
} rosidl_runtime_c__String__Sequence;

// Function prototypes from the provided source code
bool rosidl_runtime_c__String__init(rosidl_runtime_c__String *str);
void rosidl_runtime_c__String__fini(rosidl_runtime_c__String *str);
bool rosidl_runtime_c__String__copy(const rosidl_runtime_c__String *input, rosidl_runtime_c__String *output);
bool rosidl_runtime_c__String__Sequence__init(rosidl_runtime_c__String__Sequence *sequence, size_t size);
void rosidl_runtime_c__String__Sequence__fini(rosidl_runtime_c__String__Sequence *sequence);
bool rosidl_runtime_c__String__Sequence__copy(const rosidl_runtime_c__String__Sequence *input, rosidl_runtime_c__String__Sequence *output);

// Implement the functions as provided in the source code
bool rosidl_runtime_c__String__init(rosidl_runtime_c__String *str) {
    RCUTILS_CAN_RETURN_WITH_ERROR_OF(false);

    if (!str) {
        return false;
    }
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    str->data = (char *)allocator.allocate(1, allocator.state);
    if (!str->data) {
        return false;
    }
    str->data[0] = '\0';
    str->size = 0;
    str->capacity = 1;
    return true;
}

void rosidl_runtime_c__String__fini(rosidl_runtime_c__String *str) {
    if (!str) {
        return;
    }
    if (str->data) {
        /* ensure that data and capacity values are consistent */
        if (str->capacity <= 0) {
            // Using assert instead of exit for fuzzing compatibility
            assert(0 && "Unexpected condition: string capacity was zero for allocated data!");
        }
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        allocator.deallocate(str->data, allocator.state);
        str->data = NULL;
        str->size = 0;
        str->capacity = 0;
    } else {
        /* ensure that data, size, and capacity values are consistent */
        if (0 != str->size) {
            assert(0 && "Unexpected condition: string size was non-zero for deallocated data!");
        }
        if (0 != str->capacity) {
            assert(0 && "Unexpected behavior: string capacity was non-zero for deallocated data!");
        }
    }
}

bool rosidl_runtime_c__String__copy(const rosidl_runtime_c__String *input, rosidl_runtime_c__String *output) {
    if (!input || !output) {
        return false;
    }
    
    // Calculate required capacity
    size_t required_capacity = input->size + 1;
    
    // Reallocate if needed
    if (output->capacity < required_capacity) {
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        char *new_data = (char *)allocator.reallocate(output->data, required_capacity, allocator.state);
        if (!new_data) {
            return false;
        }
        output->data = new_data;
        output->capacity = required_capacity;
    }
    
    // Copy the string data
    if (input->size > 0 && input->data) {
        memcpy(output->data, input->data, input->size);
    }
    output->data[input->size] = '\0';
    output->size = input->size;
    
    return true;
}

bool rosidl_runtime_c__String__Sequence__init(rosidl_runtime_c__String__Sequence *sequence, size_t size) {
    RCUTILS_CAN_RETURN_WITH_ERROR_OF(false);

    if (!sequence) {
        return false;
    }
    rosidl_runtime_c__String *data = NULL;
    if (size) {
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        data = (rosidl_runtime_c__String *)allocator.zero_allocate(
            size, sizeof(rosidl_runtime_c__String), allocator.state);
        if (!data) {
            return false;
        }
        // initialize all sequence elements
        for (size_t i = 0; i < size; ++i) {
            if (!rosidl_runtime_c__String__init(&data[i])) {
                /* free currently allocated and return false */
                for (; i-- > 0; ) {
                    rosidl_runtime_c__String__fini(&data[i]);
                }
                allocator.deallocate(data, allocator.state);
                return false;
            }
        }
    }
    sequence->data = data;
    sequence->size = size;
    sequence->capacity = size;
    return true;
}

void rosidl_runtime_c__String__Sequence__fini(rosidl_runtime_c__String__Sequence *sequence) {
    if (!sequence) {
        return;
    }
    if (sequence->data) {
        // ensure that data and capacity values are consistent
        assert(sequence->capacity > 0);
        // finalize all sequence elements
        for (size_t i = 0; i < sequence->capacity; ++i) {
            rosidl_runtime_c__String__fini(&sequence->data[i]);
        }
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        allocator.deallocate(sequence->data, allocator.state);
        sequence->data = NULL;
        sequence->size = 0;
        sequence->capacity = 0;
    } else {
        // ensure that data, size, and capacity values are consistent
        assert(0 == sequence->size);
        assert(0 == sequence->capacity);
    }
}

bool rosidl_runtime_c__String__Sequence__copy(
    const rosidl_runtime_c__String__Sequence *input,
    rosidl_runtime_c__String__Sequence *output) {
    if (!input || !output) {
        return false;
    }
    if (output->capacity < input->size) {
        const size_t allocation_size = input->size * sizeof(rosidl_runtime_c__String);
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        rosidl_runtime_c__String *data = (rosidl_runtime_c__String *)allocator.reallocate(
            output->data, allocation_size, allocator.state);
        if (!data) {
            return false;
        }
        // If reallocation succeeded, memory may or may not have been moved
        // to fulfill the allocation request, invalidating output->data.
        output->data = data;
        for (size_t i = output->capacity; i < input->size; ++i) {
            if (!rosidl_runtime_c__String__init(&output->data[i])) {
                // If initialization of any new item fails, roll back all
                // previously initialized items. Existing items in output
                // are to be left unmodified.
                for (; i-- > output->capacity; ) {
                    rosidl_runtime_c__String__fini(&output->data[i]);
                }
                return false;
            }
        }
        output->capacity = input->size;
    }
    output->size = input->size;
    for (size_t i = 0; i < input->size; ++i) {
        if (!rosidl_runtime_c__String__copy(&(input->data[i]), &(output->data[i]))) {
            return false;
        }
    }
    return true;
}

// Fuzz driver function
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early return if input is too small for meaningful testing
    if (size < 4) {
        return 0;
    }

    // Initialize variables
    rosidl_runtime_c__String single_string = {0};
    rosidl_runtime_c__String__Sequence src_sequence = {0};
    rosidl_runtime_c__String__Sequence dst_sequence = {0};
    
    bool success = false;
    
    // 1. Test rosidl_runtime_c__String__init with a single string
    success = rosidl_runtime_c__String__init(&single_string);
    if (!success) {
        // Initialization failed, clean up and return
        goto cleanup;
    }
    
    // 2. Test rosidl_runtime_c__String__Sequence__init
    // Determine sequence size from fuzz input (bounded to prevent excessive allocation)
    size_t sequence_size = (size_t)data[0] % 16;  // Limit to 16 elements max
    if (sequence_size == 0) sequence_size = 1;    // Ensure at least 1 element
    
    success = rosidl_runtime_c__String__Sequence__init(&src_sequence, sequence_size);
    if (!success) {
        goto cleanup;
    }
    
    // 3. Populate source sequence with data from fuzz input
    size_t data_offset = 1;
    for (size_t i = 0; i < src_sequence.size && i < sequence_size; i++) {
        if (data_offset >= size) {
            break;  // No more data
        }
        
        // Determine string length from fuzz data (bounded)
        size_t str_len = (size_t)data[data_offset] % 64;  // Limit to 64 chars
        if (str_len == 0) str_len = 1;
        
        data_offset++;
        
        // Ensure we don't read beyond the input buffer
        if (data_offset + str_len > size) {
            str_len = size - data_offset;
            if (str_len == 0) break;
        }
        
        // Reallocate string buffer if needed
        if (src_sequence.data[i].capacity < str_len + 1) {
            rcutils_allocator_t allocator = rcutils_get_default_allocator();
            char *new_data = (char *)allocator.reallocate(
                src_sequence.data[i].data, str_len + 1, allocator.state);
            if (!new_data) {
                goto cleanup;
            }
            src_sequence.data[i].data = new_data;
            src_sequence.data[i].capacity = str_len + 1;
        }
        
        // Copy data into string
        if (str_len > 0) {
            memcpy(src_sequence.data[i].data, data + data_offset, str_len);
            src_sequence.data[i].data[str_len] = '\0';
            src_sequence.data[i].size = str_len;
        }
        
        data_offset += str_len;
    }
    
    // 4. Initialize destination sequence
    success = rosidl_runtime_c__String__Sequence__init(&dst_sequence, 0);
    if (!success) {
        goto cleanup;
    }
    
    // 5. Test rosidl_runtime_c__String__Sequence__copy
    success = rosidl_runtime_c__String__Sequence__copy(&src_sequence, &dst_sequence);
    if (!success) {
        goto cleanup;
    }
    
    // Verify the copy was successful by checking sizes match
    assert(src_sequence.size == dst_sequence.size);
    
    // 6. Test individual string operations on the copied data
    for (size_t i = 0; i < dst_sequence.size && i < 2; i++) {
        // Create a temporary string for testing
        rosidl_runtime_c__String temp_string = {0};
        if (rosidl_runtime_c__String__init(&temp_string)) {
            // Try to copy from destination sequence
            rosidl_runtime_c__String__copy(&dst_sequence.data[i], &temp_string);
            // Clean up temporary string
            rosidl_runtime_c__String__fini(&temp_string);
        }
    }

cleanup:
    // 7. Clean up all resources in reverse order of allocation
    // Finalize destination sequence
    rosidl_runtime_c__String__Sequence__fini(&dst_sequence);
    
    // Finalize source sequence
    rosidl_runtime_c__String__Sequence__fini(&src_sequence);
    
    // Finalize single string
    rosidl_runtime_c__String__fini(&single_string);
    
    return 0;
}

// Main function for standalone testing (not used by libFuzzer)
#ifdef STANDALONE_FUZZER
int main(int argc, char **argv) {
    // Simple test with sample data
    uint8_t test_data[] = {5, 10, 'H', 'e', 'l', 'l', 'o', 5, 'W', 'o', 'r', 'l', 'd'};
    size_t test_size = sizeof(test_data);
    
    return LLVMFuzzerTestOneInput(test_data, test_size);
}
#endif
