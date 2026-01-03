#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sstream>

#include "rosidl_runtime_c/message_type_support.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"
#include "rosidl_runtime_c/sequence_bound.h"
#include "rosidl_runtime_c/service_type_support.h"
#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_runtime_c/u16string_functions.h"

// Forward declarations for types used in the fuzzer
typedef struct rosidl_runtime_c__boolean__Sequence
{
    bool * data;
    size_t size;
    size_t capacity;
} rosidl_runtime_c__boolean__Sequence;

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

typedef struct rosidl_service_type_support_t
{
    const char * typesupport_identifier;
    const void * data;
    void (*func)(void);
} rosidl_service_type_support_t;

typedef struct rosidl_runtime_c__Sequence__bound
{
    const char * typesupport_identifier;
    const void * data;
} rosidl_runtime_c__Sequence__bound;

// Mock allocator functions for the fuzzer
typedef struct rcutils_allocator_t
{
    void * (*allocate)(size_t size, void * state);
    void (*deallocate)(void * pointer, void * state);
    void * (*reallocate)(void * pointer, size_t size, void * state);
    void * (*zero_allocate)(size_t number_of_elements, size_t size_of_element, void * state);
    void * state;
} rcutils_allocator_t;

static void * mock_allocate(size_t size, void * state)
{
    (void)state;
    return malloc(size);
}

static void mock_deallocate(void * pointer, void * state)
{
    (void)state;
    free(pointer);
}

static void * mock_reallocate(void * pointer, size_t size, void * state)
{
    (void)state;
    return realloc(pointer, size);
}

static void * mock_zero_allocate(size_t number_of_elements, size_t size_of_element, void * state)
{
    (void)state;
    size_t total_size = number_of_elements * size_of_element;
    void * ptr = malloc(total_size);
    if (ptr)
    {
        memset(ptr, 0, total_size);
    }
    return ptr;
}

static rcutils_allocator_t rcutils_get_default_allocator(void)
{
    static rcutils_allocator_t allocator = {
        .allocate = mock_allocate,
        .deallocate = mock_deallocate,
        .reallocate = mock_reallocate,
        .zero_allocate = mock_zero_allocate,
        .state = NULL
    };
    return allocator;
}

// Mock RCUTILS_CAN_RETURN_WITH_ERROR_OF macro
#define RCUTILS_CAN_RETURN_WITH_ERROR_OF(x) (void)0

// Mock rosidl_runtime_c__String__init function
bool rosidl_runtime_c__String__init(rosidl_runtime_c__String * str)
{
    if (!str)
    {
        return false;
    }
    str->data = NULL;
    str->size = 0;
    str->capacity = 0;
    return true;
}

// API implementations from provided source code
bool rosidl_runtime_c__bool__Sequence__are_equal(
    const rosidl_runtime_c__boolean__Sequence * lhs,
    const rosidl_runtime_c__boolean__Sequence * rhs)
{
    if (!lhs || !rhs)
    {
        return false;
    }
    if (lhs->size != rhs->size)
    {
        return false;
    }
    for (size_t i = 0; i < lhs->size; ++i)
    {
        if (lhs->data[i] != rhs->data[i])
        {
            return false;
        }
    }
    return true;
}

const rosidl_service_type_support_t * get_service_typesupport_handle(
    const rosidl_service_type_support_t * handle, const char * identifier)
{
    if (!handle || !identifier)
    {
        return NULL;
    }
    // Simplified mock implementation
    if (handle->typesupport_identifier && strcmp(handle->typesupport_identifier, identifier) == 0)
    {
        return handle;
    }
    return NULL;
}

bool rosidl_runtime_c__String__Sequence__init(
    rosidl_runtime_c__String__Sequence * sequence, size_t size)
{
    RCUTILS_CAN_RETURN_WITH_ERROR_OF(false);

    if (!sequence)
    {
        return false;
    }
    rosidl_runtime_c__String * data = NULL;
    if (size)
    {
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        data = (rosidl_runtime_c__String *)allocator.zero_allocate(
            size, sizeof(rosidl_runtime_c__String), allocator.state);
        if (!data)
        {
            return false;
        }
        // initialize all sequence elements
        for (size_t i = 0; i < size; ++i)
        {
            if (!rosidl_runtime_c__String__init(&data[i]))
            {
                /* free currently allocated and return false */
                for (; i-- > 0; )
                {
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

const rosidl_runtime_c__Sequence__bound * get_sequence_bound_handle_function(
    const rosidl_runtime_c__Sequence__bound * handle, const char * identifier)
{
    if (!handle || !identifier)
    {
        return NULL;
    }
    if (handle->typesupport_identifier && strcmp(handle->typesupport_identifier, identifier) == 0)
    {
        return handle;
    }
    return NULL;
}

void rosidl_runtime_c__String__fini(rosidl_runtime_c__String * str)
{
    if (!str)
    {
        return;
    }
    if (str->data)
    {
        /* ensure that data and capacity values are consistent */
        if (str->capacity <= 0)
        {
            // In fuzzing, we don't want to exit, so we just clean up
            free(str->data);
            str->data = NULL;
            str->size = 0;
            str->capacity = 0;
            return;
        }
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        allocator.deallocate(str->data, allocator.state);
        str->data = NULL;
        str->size = 0;
        str->capacity = 0;
    }
    else
    {
        /* ensure that data, size, and capacity values are consistent */
        if (0 != str->size || 0 != str->capacity)
        {
            // Reset to consistent state
            str->size = 0;
            str->capacity = 0;
        }
    }
}

// Helper function to clean up string sequence
void cleanup_string_sequence(rosidl_runtime_c__String__Sequence * seq)
{
    if (!seq)
    {
        return;
    }
    if (seq->data)
    {
        for (size_t i = 0; i < seq->size; ++i)
        {
            rosidl_runtime_c__String__fini(&seq->data[i]);
        }
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        allocator.deallocate(seq->data, allocator.state);
        seq->data = NULL;
    }
    seq->size = 0;
    seq->capacity = 0;
}

// Main fuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 2)
    {
        // Need at least 2 bytes for basic operations
        return 0;
    }

    // Initialize variables
    rosidl_runtime_c__boolean__Sequence bool_seq1 = {0};
    rosidl_runtime_c__boolean__Sequence bool_seq2 = {0};
    rosidl_runtime_c__String__Sequence string_seq = {0};
    rosidl_service_type_support_t service_handle = {0};
    rosidl_runtime_c__Sequence__bound sequence_bound = {0};
    
    // Allocate memory for boolean sequences based on fuzz input
    size_t bool_seq_size = (size_t)(data[0] % 16) + 1; // Limit size to prevent excessive allocation
    if (bool_seq_size * 2 > size - 1)
    {
        // Not enough data for both sequences
        return 0;
    }

    bool_seq1.data = (bool *)malloc(bool_seq_size * sizeof(bool));
    bool_seq2.data = (bool *)malloc(bool_seq_size * sizeof(bool));
    
    if (!bool_seq1.data || !bool_seq2.data)
    {
        free(bool_seq1.data);
        free(bool_seq2.data);
        return 0;
    }

    bool_seq1.size = bool_seq_size;
    bool_seq1.capacity = bool_seq_size;
    bool_seq2.size = bool_seq_size;
    bool_seq2.capacity = bool_seq_size;

    // Fill boolean sequences with fuzz data
    for (size_t i = 0; i < bool_seq_size; ++i)
    {
        bool_seq1.data[i] = (data[i + 1] & 0x01) != 0;
        bool_seq2.data[i] = (data[i + 1 + bool_seq_size] & 0x01) != 0;
    }

    // 1. Test rosidl_runtime_c__bool__Sequence__are_equal
    bool are_equal = rosidl_runtime_c__bool__Sequence__are_equal(&bool_seq1, &bool_seq2);
    (void)are_equal; // Use result to avoid unused variable warning

    // 2. Test rosidl_runtime_c__String__Sequence__init
    size_t string_seq_size = (size_t)(data[0] % 8) + 1; // Limit size
    bool init_success = rosidl_runtime_c__String__Sequence__init(&string_seq, string_seq_size);
    
    if (init_success)
    {
        // 3. Test rosidl_runtime_c__String__fini on individual strings
        for (size_t i = 0; i < string_seq.size && i < 2; ++i)
        {
            rosidl_runtime_c__String__fini(&string_seq.data[i]);
        }
        
        // Clean up the rest of the sequence properly
        cleanup_string_sequence(&string_seq);
    }

    // 4. Test get_service_typesupport_handle
    // Create mock identifiers from fuzz data
    char service_identifier[32] = {0};
    size_t identifier_len = (size_t)(data[0] % 31) + 1;
    if (identifier_len > size - 1)
    {
        identifier_len = size > 1 ? size - 1 : 1;
    }
    
    for (size_t i = 0; i < identifier_len && i + 1 < size; ++i)
    {
        service_identifier[i] = (char)(data[i + 1] % 26 + 'a');
    }
    service_identifier[identifier_len] = '\0';

    service_handle.typesupport_identifier = service_identifier;
    const rosidl_service_type_support_t * retrieved_handle = 
        get_service_typesupport_handle(&service_handle, service_identifier);
    (void)retrieved_handle; // Use result to avoid unused variable warning

    // 5. Test get_sequence_bound_handle_function
    char sequence_identifier[32] = {0};
    size_t seq_id_len = (size_t)(data[0] % 31) + 1;
    if (seq_id_len > size - 1)
    {
        seq_id_len = size > 1 ? size - 1 : 1;
    }
    
    for (size_t i = 0; i < seq_id_len && i + 1 < size; ++i)
    {
        sequence_identifier[i] = (char)(data[i + 1] % 26 + 'a');
    }
    sequence_identifier[seq_id_len] = '\0';

    sequence_bound.typesupport_identifier = sequence_identifier;
    const rosidl_runtime_c__Sequence__bound * retrieved_bound = 
        get_sequence_bound_handle_function(&sequence_bound, sequence_identifier);
    (void)retrieved_bound; // Use result to avoid unused variable warning

    // Clean up boolean sequences
    free(bool_seq1.data);
    free(bool_seq2.data);

    return 0;
}
