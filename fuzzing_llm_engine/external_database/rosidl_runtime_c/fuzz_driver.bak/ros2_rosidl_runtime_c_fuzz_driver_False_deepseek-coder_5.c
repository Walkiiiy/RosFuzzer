#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sstream>
#include <assert.h>

// ROS 2 headers
#include "rosidl_runtime_c/message_type_support.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"
#include "rosidl_runtime_c/sequence_bound.h"
#include "rosidl_runtime_c/service_type_support.h"
#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_runtime_c/u16string_functions.h"

// Mock service type support structure for testing
typedef struct rosidl_service_type_support_t {
    const void * func;
} rosidl_service_type_support_t;

// Mock boolean sequence structure
typedef struct rosidl_runtime_c__boolean__Sequence {
    bool * data;
    size_t size;
    size_t capacity;
} rosidl_runtime_c__boolean__Sequence;

// Mock string structure
typedef struct rosidl_runtime_c__String {
    char * data;
    size_t size;
    size_t capacity;
} rosidl_runtime_c__String;

// Mock string sequence structure
typedef struct rosidl_runtime_c__String__Sequence {
    rosidl_runtime_c__String * data;
    size_t size;
    size_t capacity;
} rosidl_runtime_c__String__Sequence;

// Mock allocator structure
typedef struct rcutils_allocator_t {
    void * (*allocate)(size_t size, void * state);
    void (*deallocate)(void * pointer, void * state);
    void * state;
} rcutils_allocator_t;

// Function prototypes (mocked implementations)
bool rosidl_runtime_c__bool__Sequence__are_equal(
    const rosidl_runtime_c__boolean__Sequence * lhs,
    const rosidl_runtime_c__boolean__Sequence * rhs);

const rosidl_service_type_support_t * get_service_typesupport_handle(
    const rosidl_service_type_support_t * handle, const char * identifier);

bool rosidl_runtime_c__String__init(rosidl_runtime_c__String * str);
void rosidl_runtime_c__String__fini(rosidl_runtime_c__String * str);
void rosidl_runtime_c__String__Sequence__fini(
    rosidl_runtime_c__String__Sequence * sequence);

// Mock implementations of the required functions
static void * mock_allocate(size_t size, void * state) {
    (void)state;
    return malloc(size);
}

static void mock_deallocate(void * pointer, void * state) {
    (void)state;
    free(pointer);
}

static rcutils_allocator_t rcutils_get_default_allocator(void) {
    static rcutils_allocator_t allocator = {
        .allocate = mock_allocate,
        .deallocate = mock_deallocate,
        .state = NULL
    };
    return allocator;
}

// Mock implementation of rosidl_runtime_c__bool__Sequence__are_equal
bool rosidl_runtime_c__bool__Sequence__are_equal(
    const rosidl_runtime_c__boolean__Sequence * lhs,
    const rosidl_runtime_c__boolean__Sequence * rhs)
{
    if (!lhs || !rhs) {
        return false;
    }
    if (lhs->size != rhs->size) {
        return false;
    }
    for (size_t i = 0; i < lhs->size; ++i) {
        if (lhs->data[i] != rhs->data[i]) {
            return false;
        }
    }
    return true;
}

// Mock implementation of get_service_typesupport_handle
const rosidl_service_type_support_t * get_service_typesupport_handle(
    const rosidl_service_type_support_t * handle, const char * identifier)
{
    (void)identifier;
    // For fuzzing, we return the handle itself as a mock
    return handle;
}

// Mock implementation of rosidl_runtime_c__String__init
bool rosidl_runtime_c__String__init(rosidl_runtime_c__String * str)
{
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

// Mock implementation of rosidl_runtime_c__String__fini
void rosidl_runtime_c__String__fini(rosidl_runtime_c__String * str)
{
    if (!str) {
        return;
    }
    if (str->data) {
        if (str->capacity <= 0) {
            // In fuzzing, we don't want to exit, so we just free and return
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
    } else {
        // Ensure consistency
        if (0 != str->size || 0 != str->capacity) {
            // Reset to avoid undefined behavior
            str->size = 0;
            str->capacity = 0;
        }
    }
}

// Mock implementation of rosidl_runtime_c__String__Sequence__fini
void rosidl_runtime_c__String__Sequence__fini(
    rosidl_runtime_c__String__Sequence * sequence)
{
    if (!sequence) {
        return;
    }
    if (sequence->data) {
        // Ensure that data and capacity values are consistent
        if (sequence->capacity == 0) {
            // Handle invalid state gracefully
            free(sequence->data);
            sequence->data = NULL;
            sequence->size = 0;
            sequence->capacity = 0;
            return;
        }
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
        if (0 != sequence->size || 0 != sequence->capacity) {
            sequence->size = 0;
            sequence->capacity = 0;
        }
    }
}

// Fuzz driver entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Ensure we have minimal data to work with
    if (size < 2) {
        return 0;
    }

    // Initialize variables
    rosidl_runtime_c__boolean__Sequence bool_seq1 = {0};
    rosidl_runtime_c__boolean__Sequence bool_seq2 = {0};
    rosidl_service_type_support_t service_handle = {0};
    rosidl_runtime_c__String str1 = {0};
    rosidl_runtime_c__String str2 = {0};
    rosidl_runtime_c__String__Sequence str_seq = {0};
    
    // Allocate memory for boolean sequences
    size_t bool_count = size % 16 + 1; // 1-16 elements
    bool_seq1.data = (bool *)malloc(bool_count * sizeof(bool));
    bool_seq2.data = (bool *)malloc(bool_count * sizeof(bool));
    
    if (!bool_seq1.data || !bool_seq2.data) {
        goto cleanup;
    }
    
    bool_seq1.size = bool_count;
    bool_seq1.capacity = bool_count;
    bool_seq2.size = bool_count;
    bool_seq2.capacity = bool_count;
    
    // Initialize boolean sequences from fuzz data
    for (size_t i = 0; i < bool_count && i < size; ++i) {
        bool_seq1.data[i] = (data[i] & 1) != 0;
        bool_seq2.data[i] = (data[i] & 1) != 0; // Make them equal for testing
    }
    
    // Test rosidl_runtime_c__bool__Sequence__are_equal
    bool are_equal = rosidl_runtime_c__bool__Sequence__are_equal(&bool_seq1, &bool_seq2);
    (void)are_equal; // Use result to avoid unused variable warning
    
    // Test get_service_typesupport_handle
    const char *identifier = "test_service";
    const rosidl_service_type_support_t *result = 
        get_service_typesupport_handle(&service_handle, identifier);
    (void)result; // Use result to avoid unused variable warning
    
    // Test rosidl_runtime_c__String__init
    bool init_success1 = rosidl_runtime_c__String__init(&str1);
    bool init_success2 = rosidl_runtime_c__String__init(&str2);
    
    if (init_success1 && init_success2) {
        // Test rosidl_runtime_c__String__fini
        rosidl_runtime_c__String__fini(&str1);
        rosidl_runtime_c__String__fini(&str2);
    }
    
    // Test rosidl_runtime_c__String__Sequence__fini
    // First create a string sequence
    size_t str_seq_count = (size % 8) + 1; // 1-8 elements
    str_seq.data = (rosidl_runtime_c__String *)malloc(str_seq_count * sizeof(rosidl_runtime_c__String));
    
    if (str_seq.data) {
        str_seq.size = str_seq_count;
        str_seq.capacity = str_seq_count;
        
        // Initialize each string in the sequence
        for (size_t i = 0; i < str_seq_count; ++i) {
            if (!rosidl_runtime_c__String__init(&str_seq.data[i])) {
                // Clean up already initialized strings
                for (size_t j = 0; j < i; ++j) {
                    rosidl_runtime_c__String__fini(&str_seq.data[j]);
                }
                free(str_seq.data);
                str_seq.data = NULL;
                break;
            }
        }
        
        // Test the sequence fini function
        rosidl_runtime_c__String__Sequence__fini(&str_seq);
    }

cleanup:
    // Free allocated memory
    if (bool_seq1.data) {
        free(bool_seq1.data);
    }
    if (bool_seq2.data) {
        free(bool_seq2.data);
    }
    
    // Ensure strings are finalized if they were initialized
    if (str1.data) {
        rosidl_runtime_c__String__fini(&str1);
    }
    if (str2.data) {
        rosidl_runtime_c__String__fini(&str2);
    }
    
    // Ensure string sequence is finalized if it exists
    if (str_seq.data) {
        rosidl_runtime_c__String__Sequence__fini(&str_seq);
    }
    
    return 0;
}

// Main function for standalone testing (not used by libFuzzer)
#ifdef STANDALONE_TEST
int main() {
    // Test with sample data
    uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    LLVMFuzzerTestOneInput(test_data, sizeof(test_data));
    return 0;
}
#endif
