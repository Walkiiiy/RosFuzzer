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

// Forward declarations for types used in the API
typedef struct rosidl_runtime_c__String
{
    char * data;
    size_t size;
    size_t capacity;
} rosidl_runtime_c__String;

typedef struct rosidl_runtime_c__String__Sequence
{
    rosidl_runtime_c__String * data;
    size_t size;
    size_t capacity;
} rosidl_runtime_c__String__Sequence;

// Mock allocator functions since we don't have the actual rcutils implementation
typedef struct rcutils_allocator_t
{
    void * (*allocate)(size_t size, void * state);
    void * (*zero_allocate)(size_t number_of_elements, size_t size_of_element, void * state);
    void (*deallocate)(void * pointer, void * state);
    void * state;
} rcutils_allocator_t;

static void * mock_allocate(size_t size, void * state)
{
    (void)state;
    return malloc(size);
}

static void * mock_zero_allocate(size_t number_of_elements, size_t size_of_element, void * state)
{
    (void)state;
    return calloc(number_of_elements, size_of_element);
}

static void mock_deallocate(void * pointer, void * state)
{
    (void)state;
    free(pointer);
}

static rcutils_allocator_t rcutils_get_default_allocator(void)
{
    static rcutils_allocator_t allocator = {
        .allocate = mock_allocate,
        .zero_allocate = mock_zero_allocate,
        .deallocate = mock_deallocate,
        .state = NULL
    };
    return allocator;
}

// Define RCUTILS_CAN_RETURN_WITH_ERROR_OF macro if not present
#ifndef RCUTILS_CAN_RETURN_WITH_ERROR_OF
#define RCUTILS_CAN_RETURN_WITH_ERROR_OF(x) (void)0
#endif

// API function declarations (from provided source code)
bool rosidl_runtime_c__String__init(rosidl_runtime_c__String * str);
bool rosidl_runtime_c__String__Sequence__init(rosidl_runtime_c__String__Sequence * sequence, size_t size);
bool rosidl_runtime_c__String__are_equal(const rosidl_runtime_c__String * lhs, const rosidl_runtime_c__String * rhs);
bool rosidl_runtime_c__String__Sequence__are_equal(const rosidl_runtime_c__String__Sequence * lhs, const rosidl_runtime_c__String__Sequence * rhs);
void rosidl_runtime_c__String__fini(rosidl_runtime_c__String * str);

// API function implementations (from provided source code)
bool rosidl_runtime_c__String__init(rosidl_runtime_c__String * str)
{
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

bool rosidl_runtime_c__String__Sequence__init(
    rosidl_runtime_c__String__Sequence * sequence, size_t size)
{
    RCUTILS_CAN_RETURN_WITH_ERROR_OF(false);

    if (!sequence) {
        return false;
    }
    rosidl_runtime_c__String * data = NULL;
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

bool rosidl_runtime_c__String__are_equal(
    const rosidl_runtime_c__String * lhs,
    const rosidl_runtime_c__String * rhs)
{
    if (!lhs || !rhs) {
        return false;
    }
    if (lhs->size != rhs->size) {
        return false;
    }
    return memcmp(lhs->data, rhs->data, lhs->size) == 0;
}

bool rosidl_runtime_c__String__Sequence__are_equal(
    const rosidl_runtime_c__String__Sequence * lhs,
    const rosidl_runtime_c__String__Sequence * rhs)
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

void rosidl_runtime_c__String__fini(rosidl_runtime_c__String * str)
{
    if (!str) {
        return;
    }
    if (str->data) {
        /* ensure that data and capacity values are consistent */
        if (str->capacity <= 0) {
            fprintf(
                stderr, "Unexpected condition: string capacity was zero for allocated data! "
                "Exiting.\n");
            exit(-1);
        }
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        allocator.deallocate(str->data, allocator.state);
        str->data = NULL;
        str->size = 0;
        str->capacity = 0;
    } else {
        /* ensure that data, size, and capacity values are consistent */
        if (0 != str->size) {
            fprintf(
                stderr, "Unexpected condition: string size was non-zero for deallocated data! "
                "Exiting.\n");
            exit(-1);
        }
        if (0 != str->capacity) {
            fprintf(
                stderr, "Unexpected behavior: string capacity was non-zero for deallocated data! "
                "Exiting.\n");
            exit(-1);
        }
    }
}

// Helper function to clean up a string sequence
static void string_sequence_fini(rosidl_runtime_c__String__Sequence * seq)
{
    if (!seq) {
        return;
    }
    if (seq->data) {
        for (size_t i = 0; i < seq->size; ++i) {
            rosidl_runtime_c__String__fini(&seq->data[i]);
        }
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        allocator.deallocate(seq->data, allocator.state);
        seq->data = NULL;
        seq->size = 0;
        seq->capacity = 0;
    }
}

// Fuzz driver entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    // Early exit if no data
    if (size == 0) {
        return 0;
    }

    // Initialize variables
    rosidl_runtime_c__String str1 = {0};
    rosidl_runtime_c__String str2 = {0};
    rosidl_runtime_c__String__Sequence seq1 = {0};
    rosidl_runtime_c__String__Sequence seq2 = {0};
    
    bool init_success = false;
    bool seq_init_success = false;
    
    // 1. Test rosidl_runtime_c__String__init
    if (!rosidl_runtime_c__String__init(&str1)) {
        goto cleanup;
    }
    init_success = true;
    
    if (!rosidl_runtime_c__String__init(&str2)) {
        goto cleanup;
    }
    
    // 2. Test rosidl_runtime_c__String__are_equal with empty strings
    bool are_equal_empty = rosidl_runtime_c__String__are_equal(&str1, &str2);
    (void)are_equal_empty; // Use result to avoid unused variable warning
    
    // 3. Create string sequences with size derived from fuzz input
    // Use first byte to determine sequence size (bounded to prevent excessive allocation)
    size_t seq_size = (size_t)(data[0] % 16) + 1; // Range: 1-16
    
    if (!rosidl_runtime_c__String__Sequence__init(&seq1, seq_size)) {
        goto cleanup;
    }
    seq_init_success = true;
    
    if (!rosidl_runtime_c__String__Sequence__init(&seq2, seq_size)) {
        goto cleanup;
    }
    
    // 4. Test rosidl_runtime_c__String__Sequence__are_equal with empty sequences
    bool seq_are_equal_empty = rosidl_runtime_c__String__Sequence__are_equal(&seq1, &seq2);
    (void)seq_are_equal_empty; // Use result to avoid unused variable warning
    
    // 5. Test with non-empty strings if we have enough data
    if (size > 1) {
        // Calculate safe copy size
        size_t copy_size = size - 1;
        if (copy_size > 0) {
            // Allocate and copy data for str1
            rcutils_allocator_t allocator = rcutils_get_default_allocator();
            char *new_data = (char *)allocator.allocate(copy_size + 1, allocator.state);
            if (new_data) {
                // Free old data
                if (str1.data) {
                    allocator.deallocate(str1.data, allocator.state);
                }
                // Copy fuzz data (skip first byte used for seq_size)
                memcpy(new_data, data + 1, copy_size);
                new_data[copy_size] = '\0';
                str1.data = new_data;
                str1.size = copy_size;
                str1.capacity = copy_size + 1;
                
                // Allocate and copy same data for str2
                char *new_data2 = (char *)allocator.allocate(copy_size + 1, allocator.state);
                if (new_data2) {
                    // Free old data
                    if (str2.data) {
                        allocator.deallocate(str2.data, allocator.state);
                    }
                    memcpy(new_data2, data + 1, copy_size);
                    new_data2[copy_size] = '\0';
                    str2.data = new_data2;
                    str2.size = copy_size;
                    str2.capacity = copy_size + 1;
                    
                    // Test equality with non-empty strings
                    bool are_equal_nonempty = rosidl_runtime_c__String__are_equal(&str1, &str2);
                    (void)are_equal_nonempty;
                    
                    // Also test with sequences if we have enough elements
                    if (seq_size > 0 && seq1.data && seq2.data) {
                        // Fill first element of each sequence with the same data
                        if (seq1.data[0].data) {
                            allocator.deallocate(seq1.data[0].data, allocator.state);
                        }
                        char *seq_data1 = (char *)allocator.allocate(copy_size + 1, allocator.state);
                        if (seq_data1) {
                            memcpy(seq_data1, data + 1, copy_size);
                            seq_data1[copy_size] = '\0';
                            seq1.data[0].data = seq_data1;
                            seq1.data[0].size = copy_size;
                            seq1.data[0].capacity = copy_size + 1;
                        }
                        
                        if (seq2.data[0].data) {
                            allocator.deallocate(seq2.data[0].data, allocator.state);
                        }
                        char *seq_data2 = (char *)allocator.allocate(copy_size + 1, allocator.state);
                        if (seq_data2) {
                            memcpy(seq_data2, data + 1, copy_size);
                            seq_data2[copy_size] = '\0';
                            seq2.data[0].data = seq_data2;
                            seq2.data[0].size = copy_size;
                            seq2.data[0].capacity = copy_size + 1;
                        }
                        
                        // Test sequence equality with modified data
                        bool seq_are_equal_modified = rosidl_runtime_c__String__Sequence__are_equal(&seq1, &seq2);
                        (void)seq_are_equal_modified;
                    }
                }
            }
        }
    }

cleanup:
    // 6. Clean up all resources using rosidl_runtime_c__String__fini
    if (init_success) {
        rosidl_runtime_c__String__fini(&str1);
        rosidl_runtime_c__String__fini(&str2);
    }
    
    if (seq_init_success) {
        string_sequence_fini(&seq1);
        string_sequence_fini(&seq2);
    }
    
    return 0;
}
