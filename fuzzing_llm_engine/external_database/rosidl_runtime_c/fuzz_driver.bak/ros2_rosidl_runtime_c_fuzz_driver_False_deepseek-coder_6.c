#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sstream>
#include <assert.h>

// ROS 2 runtime headers
#include "rosidl_runtime_c/message_type_support.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"
#include "rosidl_runtime_c/sequence_bound.h"
#include "rosidl_runtime_c/service_type_support.h"
#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_runtime_c/u16string_functions.h"

// Forward declarations for types used in the API
typedef struct rosidl_runtime_c__boolean__Sequence rosidl_runtime_c__boolean__Sequence;
typedef struct rosidl_runtime_c__double__Sequence rosidl_runtime_c__double__Sequence;
typedef struct rosidl_runtime_c__String__Sequence rosidl_runtime_c__String__Sequence;
typedef struct rosidl_runtime_c__String rosidl_runtime_c__String;
typedef struct rosidl_service_type_support_t rosidl_service_type_support_t;
typedef struct rcutils_allocator_t rcutils_allocator_t;

// Mock allocator functions since they're referenced but not provided
rcutils_allocator_t rcutils_get_default_allocator(void) {
    static rcutils_allocator_t allocator = {
        .allocate = malloc,
        .deallocate = free,
        .reallocate = realloc,
        .zero_allocate = NULL,
        .state = NULL
    };
    return allocator;
}

// Mock string initialization function
bool rosidl_runtime_c__String__init(rosidl_runtime_c__String *str) {
    if (!str) return false;
    str->data = NULL;
    str->size = 0;
    str->capacity = 0;
    return true;
}

// Mock string copy function
bool rosidl_runtime_c__String__copy(
    const rosidl_runtime_c__String *input,
    rosidl_runtime_c__String *output)
{
    if (!input || !output) return false;
    
    // Free existing output data if any
    if (output->data) {
        free(output->data);
        output->data = NULL;
    }
    
    if (input->data && input->size > 0) {
        output->data = (char*)malloc(input->size + 1);
        if (!output->data) return false;
        memcpy(output->data, input->data, input->size);
        output->data[input->size] = '\0';
        output->size = input->size;
        output->capacity = input->size + 1;
    } else {
        output->data = NULL;
        output->size = 0;
        output->capacity = 0;
    }
    return true;
}

// Mock boolean sequence equality function
bool rosidl_runtime_c__boolean__Sequence__are_equal(
    const rosidl_runtime_c__boolean__Sequence *lhs,
    const rosidl_runtime_c__boolean__Sequence *rhs)
{
    if (!lhs || !rhs) return false;
    if (lhs->size != rhs->size) return false;
    for (size_t i = 0; i < lhs->size; ++i) {
        if (lhs->data[i] != rhs->data[i]) return false;
    }
    return true;
}

// Mock double sequence finalization function
void rosidl_runtime_c__double__Sequence__fini(rosidl_runtime_c__double__Sequence *sequence) {
    if (!sequence) return;
    if (sequence->data) {
        free(sequence->data);
        sequence->data = NULL;
    }
    sequence->size = 0;
    sequence->capacity = 0;
}

// API functions from the provided source code
bool rosidl_runtime_c__bool__Sequence__are_equal(
    const rosidl_runtime_c__boolean__Sequence *lhs,
    const rosidl_runtime_c__boolean__Sequence *rhs)
{
    return rosidl_runtime_c__boolean__Sequence__are_equal(lhs, rhs);
}

void rosidl_runtime_c__float64__Sequence__fini(
    rosidl_runtime_c__double__Sequence *sequence)
{
    rosidl_runtime_c__double__Sequence__fini(sequence);
}

const rosidl_service_type_support_t *get_service_typesupport_handle_function(
    const rosidl_service_type_support_t *handle, const char *identifier)
{
    assert(handle);
    assert(handle->typesupport_identifier);
    assert(identifier);
    if (strcmp(handle->typesupport_identifier, identifier) == 0) {
        return handle;
    }
    return 0;
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
        output->data = data;
        for (size_t i = output->capacity; i < input->size; ++i) {
            if (!rosidl_runtime_c__String__init(&output->data[i])) {
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
        if (str->capacity <= 0) {
            fprintf(stderr, "Unexpected condition: string capacity was zero for allocated data! Exiting.\n");
            exit(-1);
        }
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        allocator.deallocate(str->data, allocator.state);
        str->data = NULL;
        str->size = 0;
        str->capacity = 0;
    } else {
        if (0 != str->size) {
            fprintf(stderr, "Unexpected condition: string size was non-zero for deallocated data! Exiting.\n");
            exit(-1);
        }
        if (0 != str->capacity) {
            fprintf(stderr, "Unexpected behavior: string capacity was non-zero for deallocated data! Exiting.\n");
            exit(-1);
        }
    }
}

// Fuzz driver entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early exit for insufficient data
    if (size < 4) return 0;
    
    // Initialize random seed from fuzz data
    uint32_t seed = 0;
    memcpy(&seed, data, sizeof(seed) < size ? sizeof(seed) : size);
    srand(seed);
    
    // Use a portion of data for string operations
    size_t data_offset = 0;
    
    // 1. Test rosidl_runtime_c__bool__Sequence__are_equal
    rosidl_runtime_c__boolean__Sequence bool_seq1 = {0};
    rosidl_runtime_c__boolean__Sequence bool_seq2 = {0};
    
    // Determine sequence sizes from fuzz data
    size_t bool_seq_size = (size > 8) ? (data[4] % 16) : 4;
    if (bool_seq_size > 0) {
        bool_seq1.data = (bool*)malloc(bool_seq_size * sizeof(bool));
        bool_seq2.data = (bool*)malloc(bool_seq_size * sizeof(bool));
        if (bool_seq1.data && bool_seq2.data) {
            bool_seq1.size = bool_seq_size;
            bool_seq1.capacity = bool_seq_size;
            bool_seq2.size = bool_seq_size;
            bool_seq2.capacity = bool_seq_size;
            
            // Fill sequences with data
            for (size_t i = 0; i < bool_seq_size; ++i) {
                bool value = (i < size - 8) ? (data[8 + i] % 2) : (i % 2);
                bool_seq1.data[i] = value;
                bool_seq2.data[i] = value;  // Make them equal
            }
            
            // Test equality
            bool are_equal = rosidl_runtime_c__bool__Sequence__are_equal(&bool_seq1, &bool_seq2);
            assert(are_equal);  // Should be equal
            
            // Test with different sequences
            if (bool_seq_size > 0) {
                bool_seq2.data[0] = !bool_seq2.data[0];
                are_equal = rosidl_runtime_c__bool__Sequence__are_equal(&bool_seq1, &bool_seq2);
                assert(!are_equal);  // Should not be equal
            }
        }
        
        // Clean up
        if (bool_seq1.data) free(bool_seq1.data);
        if (bool_seq2.data) free(bool_seq2.data);
    }
    
    // 2. Test rosidl_runtime_c__float64__Sequence__fini
    rosidl_runtime_c__double__Sequence double_seq = {0};
    size_t double_seq_size = (size > 16) ? (data[12] % 8) : 2;
    if (double_seq_size > 0) {
        double_seq.data = (double*)malloc(double_seq_size * sizeof(double));
        if (double_seq.data) {
            double_seq.size = double_seq_size;
            double_seq.capacity = double_seq_size;
            
            // Fill with data
            for (size_t i = 0; i < double_seq_size && (16 + i * 8) < size; ++i) {
                double value = 0.0;
                memcpy(&value, data + 16 + i * 8, sizeof(double) < (size - (16 + i * 8)) ? sizeof(double) : (size - (16 + i * 8)));
                double_seq.data[i] = value;
            }
            
            // Finalize the sequence
            rosidl_runtime_c__float64__Sequence__fini(&double_seq);
            assert(double_seq.data == NULL);
            assert(double_seq.size == 0);
            assert(double_seq.capacity == 0);
        }
    }
    
    // 3. Test get_service_typesupport_handle_function
    // Create mock service type support structure
    rosidl_service_type_support_t mock_handle = {0};
    const char* identifier1 = "rosidl_typesupport_c";
    const char* identifier2 = "rosidl_typesupport_fastrtps_c";
    
    mock_handle.typesupport_identifier = identifier1;
    
    // Test matching identifier
    const rosidl_service_type_support_t* result = 
        get_service_typesupport_handle_function(&mock_handle, identifier1);
    assert(result == &mock_handle);
    
    // Test non-matching identifier
    result = get_service_typesupport_handle_function(&mock_handle, identifier2);
    assert(result == 0);
    
    // 4. Test rosidl_runtime_c__String__Sequence__copy and rosidl_runtime_c__String__fini
    rosidl_runtime_c__String__Sequence str_seq_input = {0};
    rosidl_runtime_c__String__Sequence str_seq_output = {0};
    
    size_t str_seq_size = (size > 24) ? (data[20] % 4) : 1;
    if (str_seq_size > 0) {
        // Initialize input sequence
        str_seq_input.data = (rosidl_runtime_c__String*)malloc(str_seq_size * sizeof(rosidl_runtime_c__String));
        if (str_seq_input.data) {
            str_seq_input.size = str_seq_size;
            str_seq_input.capacity = str_seq_size;
            
            // Initialize output sequence with smaller capacity to test reallocation
            str_seq_output.data = (rosidl_runtime_c__String*)malloc((str_seq_size / 2) * sizeof(rosidl_runtime_c__String));
            if (str_seq_output.data) {
                str_seq_output.size = str_seq_size / 2;
                str_seq_output.capacity = str_seq_size / 2;
                
                // Initialize input strings
                for (size_t i = 0; i < str_seq_size; ++i) {
                    if (!rosidl_runtime_c__String__init(&str_seq_input.data[i])) {
                        // Clean up on failure
                        for (size_t j = 0; j < i; ++j) {
                            rosidl_runtime_c__String__fini(&str_seq_input.data[j]);
                        }
                        free(str_seq_input.data);
                        free(str_seq_output.data);
                        return 0;
                    }
                    
                    // Create small strings from fuzz data
                    size_t str_len = (size > 32 + i) ? (data[24 + i] % 16) : 4;
                    if (str_len > 0 && (32 + i * 16) < size) {
                        str_seq_input.data[i].data = (char*)malloc(str_len + 1);
                        if (str_seq_input.data[i].data) {
                            size_t copy_len = str_len < (size - (32 + i * 16)) ? str_len : (size - (32 + i * 16));
                            if (copy_len > 0) {
                                memcpy(str_seq_input.data[i].data, data + 32 + i * 16, copy_len);
                            }
                            // Null-terminate
                            str_seq_input.data[i].data[str_len] = '\0';
                            str_seq_input.data[i].size = str_len;
                            str_seq_input.data[i].capacity = str_len + 1;
                        }
                    }
                }
                
                // Initialize output strings
                for (size_t i = 0; i < str_seq_size / 2; ++i) {
                    rosidl_runtime_c__String__init(&str_seq_output.data[i]);
                }
                
                // Test copy function
                bool copy_success = rosidl_runtime_c__String__Sequence__copy(&str_seq_input, &str_seq_output);
                
                if (copy_success) {
                    // Verify copy was successful
                    assert(str_seq_output.size == str_seq_input.size);
                    
                    // Clean up output strings
                    for (size_t i = 0; i < str_seq_output.size; ++i) {
                        rosidl_runtime_c__String__fini(&str_seq_output.data[i]);
                    }
                }
                
                // Clean up input strings
                for (size_t i = 0; i < str_seq_input.size; ++i) {
                    rosidl_runtime_c__String__fini(&str_seq_input.data[i]);
                }
            }
            
            free(str_seq_input.data);
            if (str_seq_output.data) free(str_seq_output.data);
        }
    }
    
    return 0;
}
