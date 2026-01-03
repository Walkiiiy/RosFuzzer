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

// Forward declarations for types used in the API
typedef struct rcutils_allocator_s rcutils_allocator_t;
typedef struct rosidl_runtime_c__String__Sequence rosidl_runtime_c__String__Sequence;
typedef struct rosidl_runtime_c__String rosidl_runtime_c__String;

// Mock allocator functions since they're referenced but not defined in provided headers
rcutils_allocator_t rcutils_get_default_allocator(void) {
    static rcutils_allocator_t allocator = {
        .allocate = malloc,
        .deallocate = free,
        .reallocate = realloc,
        .zero_allocate = calloc,
        .state = NULL
    };
    return allocator;
}

// Mock string equality function referenced in are_equal
bool rosidl_runtime_c__String__are_equal(
    const rosidl_runtime_c__String *lhs,
    const rosidl_runtime_c__String *rhs) {
    if (!lhs || !rhs) return false;
    if (lhs->size != rhs->size) return false;
    return memcmp(lhs->data, rhs->data, lhs->size) == 0;
}

// Mock string copy function referenced in sequence copy
bool rosidl_runtime_c__String__copy(
    const rosidl_runtime_c__String *input,
    rosidl_runtime_c__String *output) {
    if (!input || !output) return false;
    
    // Ensure output is initialized
    if (!rosidl_runtime_c__String__init(output)) {
        return false;
    }
    
    // Allocate memory for the string data
    if (input->size > 0) {
        output->data = malloc(input->size);
        if (!output->data) {
            rosidl_runtime_c__String__fini(output);
            return false;
        }
        memcpy(output->data, input->data, input->size);
        output->size = input->size;
        output->capacity = input->size;
    }
    return true;
}

// Mock string init function referenced in sequence init
bool rosidl_runtime_c__String__init(rosidl_runtime_c__String *str) {
    if (!str) return false;
    str->data = NULL;
    str->size = 0;
    str->capacity = 0;
    return true;
}

// The actual API functions (provided in source code)
bool rosidl_runtime_c__String__Sequence__are_equal(
    const rosidl_runtime_c__String__Sequence *lhs,
    const rosidl_runtime_c__String__Sequence *rhs);

bool rosidl_runtime_c__String__Sequence__copy(
    const rosidl_runtime_c__String__Sequence *input,
    rosidl_runtime_c__String__Sequence *output);

void rosidl_runtime_c__String__fini(rosidl_runtime_c__String *str);

bool rosidl_runtime_c__String__Sequence__init(
    rosidl_runtime_c__String__Sequence *sequence, size_t size);

void rosidl_runtime_c__String__Sequence__fini(
    rosidl_runtime_c__String__Sequence *sequence);

// Fuzz driver entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early return if insufficient data
    if (size < 2) {
        return 0;
    }

    // Use first byte to determine sequence size (limit to reasonable value)
    size_t seq_size = (data[0] % 16) + 1;  // 1-16 elements
    if (seq_size * 2 > size - 1) {
        // Not enough data for even minimal strings
        return 0;
    }

    // Initialize first sequence
    rosidl_runtime_c__String__Sequence seq1;
    if (!rosidl_runtime_c__String__Sequence__init(&seq1, seq_size)) {
        return 0;
    }

    // Fill sequence with data from fuzz input
    const uint8_t *data_ptr = data + 1;
    size_t remaining = size - 1;
    
    for (size_t i = 0; i < seq_size && remaining > 0; i++) {
        // Determine string length (1-255 bytes, but limited by remaining data)
        size_t str_len = (data_ptr[0] % 16) + 1;  // 1-16 bytes
        if (str_len > remaining - 1) {
            str_len = remaining - 1;
        }
        if (str_len == 0) {
            break;
        }
        
        // Allocate and copy string data
        seq1.data[i].data = malloc(str_len);
        if (!seq1.data[i].data) {
            // Clean up and exit on allocation failure
            rosidl_runtime_c__String__Sequence__fini(&seq1);
            return 0;
        }
        memcpy(seq1.data[i].data, data_ptr + 1, str_len);
        seq1.data[i].size = str_len;
        seq1.data[i].capacity = str_len;
        
        // Move data pointer forward
        size_t consumed = 1 + str_len;
        data_ptr += consumed;
        remaining = (remaining > consumed) ? remaining - consumed : 0;
    }

    // Initialize second sequence with same size
    rosidl_runtime_c__String__Sequence seq2;
    if (!rosidl_runtime_c__String__Sequence__init(&seq2, seq_size)) {
        rosidl_runtime_c__String__Sequence__fini(&seq1);
        return 0;
    }

    // Test rosidl_runtime_c__String__Sequence__copy
    bool copy_success = rosidl_runtime_c__String__Sequence__copy(&seq1, &seq2);
    if (!copy_success) {
        // Copy failed, clean up and exit
        rosidl_runtime_c__String__Sequence__fini(&seq1);
        rosidl_runtime_c__String__Sequence__fini(&seq2);
        return 0;
    }

    // Test rosidl_runtime_c__String__Sequence__are_equal
    // Should return true after successful copy
    bool are_equal = rosidl_runtime_c__String__Sequence__are_equal(&seq1, &seq2);
    (void)are_equal;  // Use result to avoid unused variable warning

    // Test individual string finalization on a few elements
    for (size_t i = 0; i < 2 && i < seq_size; i++) {
        rosidl_runtime_c__String__fini(&seq1.data[i]);
        // Re-initialize for proper cleanup later
        rosidl_runtime_c__String__init(&seq1.data[i]);
    }

    // Clean up all resources
    rosidl_runtime_c__String__Sequence__fini(&seq1);
    rosidl_runtime_c__String__Sequence__fini(&seq2);

    return 0;
}

// Implement the actual API functions (copied from provided source)
bool rosidl_runtime_c__String__Sequence__are_equal(
    const rosidl_runtime_c__String__Sequence *lhs,
    const rosidl_runtime_c__String__Sequence *rhs)
{
    if (!lhs || !rhs) {
        return false;
    }
    if (lhs->size != rhs->size) {
        return false;
    }
    for (size_t i = 0; i < lhs->size; ++i) {
        if (!rosidl_runtime_c__String__are_equal(
            &(lhs->data[i]), &(rhs->data[i])))
        {
            return false;
        }
    }
    return true;
}

bool rosidl_runtime_c__String__Sequence__copy(
    const rosidl_runtime_c__String__Sequence *input,
    rosidl_runtime_c__String__Sequence *output)
{
    if (!input || !output) {
        return false;
    }
    if (output->capacity < input->size) {
        const size_t allocation_size =
            input->size * sizeof(rosidl_runtime_c__String);
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        rosidl_runtime_c__String *data =
            (rosidl_runtime_c__String *)allocator.reallocate(
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
        if (!rosidl_runtime_c__String__copy(
            &(input->data[i]), &(output->data[i])))
        {
            return false;
        }
    }
    return true;
}

void rosidl_runtime_c__String__fini(rosidl_runtime_c__String *str)
{
    if (!str) {
        return;
    }
    if (str->data) {
        /* ensure that data and capacity values are consistent */
        if (str->capacity <= 0) {
            // Use assert instead of exit for fuzzing
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

bool rosidl_runtime_c__String__Sequence__init(
    rosidl_runtime_c__String__Sequence *sequence, size_t size)
{
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

void rosidl_runtime_c__String__Sequence__fini(
    rosidl_runtime_c__String__Sequence *sequence)
{
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
