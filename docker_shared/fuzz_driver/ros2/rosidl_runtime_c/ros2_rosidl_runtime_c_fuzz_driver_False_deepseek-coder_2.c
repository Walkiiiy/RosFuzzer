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

// Mock structures for types that aren't fully defined in headers
typedef struct rosidl_message_type_support_s {
    const char * typesupport_identifier;
} rosidl_message_type_support_t;

typedef struct rosidl_service_type_support_s {
    void * func;
} rosidl_service_type_support_t;

// Mock function for rosidl_service_typesupport_handle_function
typedef const rosidl_service_type_support_t * (*rosidl_service_typesupport_handle_function)(
    const rosidl_service_type_support_t *, const char *);

// Mock implementation of get_service_typesupport_handle
const rosidl_service_type_support_t * get_service_typesupport_handle(
    const rosidl_service_type_support_t * handle, const char * identifier)
{
    assert(handle);
    assert(handle->func);
    rosidl_service_typesupport_handle_function func =
        (rosidl_service_typesupport_handle_function)(handle->func);
    return func(handle, identifier);
}

// Mock implementation of get_message_typesupport_handle_function
const rosidl_message_type_support_t * get_message_typesupport_handle_function(
    const rosidl_message_type_support_t * handle, const char * identifier)
{
    assert(handle);
    assert(handle->typesupport_identifier);
    assert(identifier);
    if (strcmp(handle->typesupport_identifier, identifier) == 0) {
        return handle;
    }
    return 0;
}

// Mock allocator structure and functions
typedef struct rcutils_allocator_s {
    void * (*allocate)(size_t size, void * state);
    void (*deallocate)(void * pointer, void * state);
    void * (*reallocate)(void * pointer, size_t size, void * state);
    void * (*zero_allocate)(size_t number_of_elements, size_t size_of_element, void * state);
    void * state;
} rcutils_allocator_t;

static void * mock_allocate(size_t size, void * state) {
    (void)state;
    return malloc(size);
}

static void mock_deallocate(void * pointer, void * state) {
    (void)state;
    free(pointer);
}

static void * mock_zero_allocate(size_t number_of_elements, size_t size_of_element, void * state) {
    (void)state;
    size_t total_size = number_of_elements * size_of_element;
    if (total_size == 0) return NULL;
    void * ptr = calloc(1, total_size);
    return ptr;
}

rcutils_allocator_t rcutils_get_default_allocator(void) {
    static rcutils_allocator_t allocator = {
        .allocate = mock_allocate,
        .deallocate = mock_deallocate,
        .reallocate = NULL,
        .zero_allocate = mock_zero_allocate,
        .state = NULL
    };
    return allocator;
}

// Mock string and sequence structures
typedef struct rosidl_runtime_c__String_s {
    char * data;
    size_t size;
    size_t capacity;
} rosidl_runtime_c__String;

typedef struct rosidl_runtime_c__String__Sequence_s {
    rosidl_runtime_c__String * data;
    size_t size;
    size_t capacity;
} rosidl_runtime_c__String__Sequence;

typedef struct rosidl_runtime_c__boolean__Sequence_s {
    bool * data;
    size_t size;
    size_t capacity;
} rosidl_runtime_c__boolean__Sequence;

// Mock string functions
bool rosidl_runtime_c__String__init(rosidl_runtime_c__String * str) {
    if (!str) return false;
    str->data = NULL;
    str->size = 0;
    str->capacity = 0;
    return true;
}

void rosidl_runtime_c__String__fini(rosidl_runtime_c__String * str) {
    if (str && str->data) {
        free(str->data);
        str->data = NULL;
        str->size = 0;
        str->capacity = 0;
    }
}

bool rosidl_runtime_c__String__are_equal(
    const rosidl_runtime_c__String * lhs,
    const rosidl_runtime_c__String * rhs)
{
    if (!lhs || !rhs) return false;
    if (lhs->size != rhs->size) return false;
    if (lhs->size == 0) return true;
    return memcmp(lhs->data, rhs->data, lhs->size) == 0;
}

// Mock sequence initialization function
bool rosidl_runtime_c__String__Sequence__init(
    rosidl_runtime_c__String__Sequence * sequence, size_t size)
{
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
                // free currently allocated and return false
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

// Mock sequence equality functions
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

bool rosidl_runtime_c__bool__Sequence__are_equal(
    const rosidl_runtime_c__boolean__Sequence * lhs,
    const rosidl_runtime_c__boolean__Sequence * rhs)
{
    if (!lhs || !rhs) return false;
    if (lhs->size != rhs->size) return false;
    if (lhs->size == 0) return true;
    return memcmp(lhs->data, rhs->data, lhs->size * sizeof(bool)) == 0;
}

// Cleanup function for string sequence
void rosidl_runtime_c__String__Sequence__fini(rosidl_runtime_c__String__Sequence * sequence) {
    if (sequence && sequence->data) {
        for (size_t i = 0; i < sequence->size; ++i) {
            rosidl_runtime_c__String__fini(&sequence->data[i]);
        }
        free(sequence->data);
        sequence->data = NULL;
        sequence->size = 0;
        sequence->capacity = 0;
    }
}

// Cleanup function for boolean sequence
void rosidl_runtime_c__bool__Sequence__fini(rosidl_runtime_c__boolean__Sequence * sequence) {
    if (sequence && sequence->data) {
        free(sequence->data);
        sequence->data = NULL;
        sequence->size = 0;
        sequence->capacity = 0;
    }
}

// Fuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Ensure we have minimum data for operations
    if (size < 4) return 0;
    
    // Initialize variables
    rosidl_runtime_c__String__Sequence seq1 = {0};
    rosidl_runtime_c__String__Sequence seq2 = {0};
    rosidl_runtime_c__boolean__Sequence bool_seq1 = {0};
    rosidl_runtime_c__boolean__Sequence bool_seq2 = {0};
    
    // Use first byte to determine sequence sizes (limit to reasonable values)
    size_t seq_size = (data[0] % 16) + 1;  // 1-16 elements
    size_t bool_seq_size = (data[1] % 16) + 1;  // 1-16 elements
    
    // Initialize string sequences
    if (!rosidl_runtime_c__String__Sequence__init(&seq1, seq_size)) {
        goto cleanup;
    }
    
    if (!rosidl_runtime_c__String__Sequence__init(&seq2, seq_size)) {
        rosidl_runtime_c__String__Sequence__fini(&seq1);
        goto cleanup;
    }
    
    // Initialize boolean sequences
    bool_seq1.data = (bool *)calloc(bool_seq_size, sizeof(bool));
    bool_seq1.size = bool_seq_size;
    bool_seq1.capacity = bool_seq_size;
    
    bool_seq2.data = (bool *)calloc(bool_seq_size, sizeof(bool));
    bool_seq2.size = bool_seq_size;
    bool_seq2.capacity = bool_seq_size;
    
    if (!bool_seq1.data || !bool_seq2.data) {
        goto cleanup;
    }
    
    // Fill boolean sequences with fuzz data
    size_t bool_data_offset = 2;
    for (size_t i = 0; i < bool_seq_size && bool_data_offset < size; i++) {
        bool_seq1.data[i] = (data[bool_data_offset] & 1) != 0;
        bool_seq2.data[i] = (data[bool_data_offset] & 2) != 0;
        bool_data_offset++;
    }
    
    // Fill string sequences with fuzz data
    size_t str_data_offset = bool_data_offset;
    for (size_t i = 0; i < seq_size && str_data_offset < size; i++) {
        // Determine string length (1-16 bytes)
        size_t str_len = (data[str_data_offset] % 16) + 1;
        str_data_offset++;
        
        // Allocate and fill string data
        if (str_data_offset + str_len <= size) {
            // For seq1
            seq1.data[i].data = (char *)malloc(str_len + 1);
            if (seq1.data[i].data) {
                memcpy(seq1.data[i].data, data + str_data_offset, str_len);
                seq1.data[i].data[str_len] = '\0';
                seq1.data[i].size = str_len;
                seq1.data[i].capacity = str_len + 1;
            }
            
            // For seq2 - sometimes make it equal, sometimes different
            if (i % 2 == 0 && seq1.data[i].data) {
                // Make equal
                seq2.data[i].data = (char *)malloc(str_len + 1);
                if (seq2.data[i].data) {
                    memcpy(seq2.data[i].data, data + str_data_offset, str_len);
                    seq2.data[i].data[str_len] = '\0';
                    seq2.data[i].size = str_len;
                    seq2.data[i].capacity = str_len + 1;
                }
            } else {
                // Make different by using different data
                size_t diff_offset = (str_data_offset + 1) % size;
                size_t diff_len = str_len;
                if (diff_offset + diff_len > size) {
                    diff_len = size - diff_offset;
                }
                if (diff_len > 0) {
                    seq2.data[i].data = (char *)malloc(diff_len + 1);
                    if (seq2.data[i].data) {
                        memcpy(seq2.data[i].data, data + diff_offset, diff_len);
                        seq2.data[i].data[diff_len] = '\0';
                        seq2.data[i].size = diff_len;
                        seq2.data[i].capacity = diff_len + 1;
                    }
                }
            }
            
            str_data_offset += str_len;
        }
    }
    
    // Test 1: Compare string sequences
    bool strings_equal = rosidl_runtime_c__String__Sequence__are_equal(&seq1, &seq2);
    (void)strings_equal; // Result used for fuzzing exploration
    
    // Test 2: Compare boolean sequences
    bool bools_equal = rosidl_runtime_c__bool__Sequence__are_equal(&bool_seq1, &bool_seq2);
    (void)bools_equal; // Result used for fuzzing exploration
    
    // Test 3: Test message typesupport handle function
    rosidl_message_type_support_t msg_handle = {
        .typesupport_identifier = "rosidl_typesupport_c"
    };
    
    const char * identifier = "rosidl_typesupport_c";
    const rosidl_message_type_support_t * msg_result = 
        get_message_typesupport_handle_function(&msg_handle, identifier);
    (void)msg_result;
    
    // Test with different identifier
    const rosidl_message_type_support_t * msg_result2 = 
        get_message_typesupport_handle_function(&msg_handle, "different_id");
    (void)msg_result2;
    
    // Test 4: Test service typesupport handle
    // Create a mock function that returns the handle itself
    static const rosidl_service_type_support_t * mock_service_func(
        const rosidl_service_type_support_t * handle, const char * id) {
        (void)id;
        return handle;
    }
    
    rosidl_service_type_support_t service_handle = {
        .func = (void *)mock_service_func
    };
    
    const rosidl_service_type_support_t * service_result = 
        get_service_typesupport_handle(&service_handle, "test_identifier");
    (void)service_result;
    
    // Test with NULL inputs for robustness
    bool null_test1 = rosidl_runtime_c__String__Sequence__are_equal(NULL, &seq1);
    bool null_test2 = rosidl_runtime_c__String__Sequence__are_equal(&seq1, NULL);
    bool null_test3 = rosidl_runtime_c__String__Sequence__are_equal(NULL, NULL);
    (void)null_test1;
    (void)null_test2;
    (void)null_test3;
    
    bool null_test4 = rosidl_runtime_c__bool__Sequence__are_equal(NULL, &bool_seq1);
    bool null_test5 = rosidl_runtime_c__bool__Sequence__are_equal(&bool_seq1, NULL);
    bool null_test6 = rosidl_runtime_c__bool__Sequence__are_equal(NULL, NULL);
    (void)null_test4;
    (void)null_test5;
    (void)null_test6;
    
    // Test initialization with zero size
    rosidl_runtime_c__String__Sequence zero_seq = {0};
    bool init_zero = rosidl_runtime_c__String__Sequence__init(&zero_seq, 0);
    if (init_zero) {
        rosidl_runtime_c__String__Sequence__fini(&zero_seq);
    }
    
cleanup:
    // Clean up all allocated resources
    rosidl_runtime_c__String__Sequence__fini(&seq1);
    rosidl_runtime_c__String__Sequence__fini(&seq2);
    
    if (bool_seq1.data) {
        free(bool_seq1.data);
    }
    if (bool_seq2.data) {
        free(bool_seq2.data);
    }
    
    return 0;
}
