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
    void (*deallocate)(void * pointer, void * state);
    void * (*reallocate)(void * pointer, size_t size, void * state);
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

static rcutils_allocator_t rcutils_get_default_allocator(void)
{
    static rcutils_allocator_t allocator = {
        .allocate = mock_allocate,
        .deallocate = mock_deallocate,
        .reallocate = mock_reallocate,
        .state = NULL
    };
    return allocator;
}

// API function declarations (implemented below based on provided source)
bool rosidl_runtime_c__String__Sequence__are_equal(
    const rosidl_runtime_c__String__Sequence * lhs,
    const rosidl_runtime_c__String__Sequence * rhs);

bool rosidl_runtime_c__String__init(rosidl_runtime_c__String * str);

bool rosidl_runtime_c__String__are_equal(
    const rosidl_runtime_c__String * lhs,
    const rosidl_runtime_c__String * rhs);

void rosidl_runtime_c__String__fini(rosidl_runtime_c__String * str);

bool rosidl_runtime_c__String__assignn(
    rosidl_runtime_c__String * str, const char * value, size_t n);

// API implementations (from provided source code)
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

bool rosidl_runtime_c__String__assignn(
    rosidl_runtime_c__String * str, const char * value, size_t n)
{
    if (!str) {
        return false;
    }
    // a NULL value is not valid
    if (!value) {
        return false;
    }
    // since n + 1 bytes are being allocated n can't be the maximum value
    if (n == SIZE_MAX) {
        return false;
    }
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    char * data = (char *)allocator.reallocate(str->data, n + 1, allocator.state);
    if (!data) {
        return false;
    }
    memcpy(data, value, n);
    data[n] = '\0';
    str->data = data;
    str->size = n;
    str->capacity = n + 1;
    return true;
}

// Helper function to create a sequence from fuzz data
static rosidl_runtime_c__String__Sequence * create_string_sequence_from_fuzz(
    const uint8_t *data, size_t size, size_t num_strings)
{
    if (num_strings == 0 || size < num_strings) {
        return NULL;
    }
    
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    rosidl_runtime_c__String__Sequence *seq = 
        (rosidl_runtime_c__String__Sequence *)allocator.allocate(
            sizeof(rosidl_runtime_c__String__Sequence), allocator.state);
    if (!seq) {
        return NULL;
    }
    
    seq->data = (rosidl_runtime_c__String *)allocator.allocate(
        sizeof(rosidl_runtime_c__String) * num_strings, allocator.state);
    if (!seq->data) {
        allocator.deallocate(seq, allocator.state);
        return NULL;
    }
    
    seq->size = num_strings;
    seq->capacity = num_strings;
    
    // Initialize all strings
    for (size_t i = 0; i < num_strings; i++) {
        if (!rosidl_runtime_c__String__init(&seq->data[i])) {
            // Clean up initialized strings
            for (size_t j = 0; j < i; j++) {
                rosidl_runtime_c__String__fini(&seq->data[j]);
            }
            allocator.deallocate(seq->data, allocator.state);
            allocator.deallocate(seq, allocator.state);
            return NULL;
        }
    }
    
    // Assign data to strings (distribute fuzz data among strings)
    size_t bytes_per_string = size / num_strings;
    const uint8_t *current_data = data;
    size_t remaining_size = size;
    
    for (size_t i = 0; i < num_strings; i++) {
        size_t string_len = (i < num_strings - 1) ? bytes_per_string : remaining_size;
        if (string_len > 0) {
            // Ensure we don't overflow
            if (string_len > remaining_size) {
                string_len = remaining_size;
            }
            
            // Create a temporary buffer for the string data
            char *temp_str = (char *)allocator.allocate(string_len + 1, allocator.state);
            if (!temp_str) {
                // Clean up on failure
                for (size_t j = 0; j < num_strings; j++) {
                    rosidl_runtime_c__String__fini(&seq->data[j]);
                }
                allocator.deallocate(seq->data, allocator.state);
                allocator.deallocate(seq, allocator.state);
                return NULL;
            }
            
            memcpy(temp_str, current_data, string_len);
            temp_str[string_len] = '\0';
            
            if (!rosidl_runtime_c__String__assignn(&seq->data[i], temp_str, string_len)) {
                allocator.deallocate(temp_str, allocator.state);
                for (size_t j = 0; j < num_strings; j++) {
                    rosidl_runtime_c__String__fini(&seq->data[j]);
                }
                allocator.deallocate(seq->data, allocator.state);
                allocator.deallocate(seq, allocator.state);
                return NULL;
            }
            
            allocator.deallocate(temp_str, allocator.state);
            current_data += string_len;
            remaining_size -= string_len;
        }
    }
    
    return seq;
}

// Helper function to free a sequence
static void free_string_sequence(rosidl_runtime_c__String__Sequence *seq)
{
    if (!seq) {
        return;
    }
    
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    
    if (seq->data) {
        for (size_t i = 0; i < seq->size; i++) {
            rosidl_runtime_c__String__fini(&seq->data[i]);
        }
        allocator.deallocate(seq->data, allocator.state);
    }
    
    allocator.deallocate(seq, allocator.state);
}

// Main fuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    // Skip empty inputs
    if (size == 0) {
        return 0;
    }
    
    // Use first byte to determine number of strings (1-16)
    size_t num_strings = (data[0] % 16) + 1;
    
    // Adjust data pointer and size for the rest of the input
    const uint8_t *string_data = data + 1;
    size_t string_data_size = (size > 1) ? size - 1 : 0;
    
    // Create two sequences from the same fuzz data
    rosidl_runtime_c__String__Sequence *seq1 = 
        create_string_sequence_from_fuzz(string_data, string_data_size, num_strings);
    if (!seq1) {
        return 0;
    }
    
    rosidl_runtime_c__String__Sequence *seq2 = 
        create_string_sequence_from_fuzz(string_data, string_data_size, num_strings);
    if (!seq2) {
        free_string_sequence(seq1);
        return 0;
    }
    
    // Test 1: Compare sequences (should be equal since created from same data)
    bool sequences_equal = rosidl_runtime_c__String__Sequence__are_equal(seq1, seq2);
    (void)sequences_equal; // Use result to avoid unused variable warning
    
    // Test 2: Compare individual strings within sequences
    for (size_t i = 0; i < seq1->size && i < seq2->size; i++) {
        bool strings_equal = rosidl_runtime_c__String__are_equal(
            &seq1->data[i], &seq2->data[i]);
        (void)strings_equal; // Use result to avoid unused variable warning
    }
    
    // Test 3: Create and manipulate individual strings
    rosidl_runtime_c__String single_str1, single_str2;
    
    // Initialize strings
    if (rosidl_runtime_c__String__init(&single_str1) &&
        rosidl_runtime_c__String__init(&single_str2)) {
        
        // Assign data to strings (use a portion of fuzz data)
        size_t assign_len = (string_data_size > 100) ? 100 : string_data_size;
        if (assign_len > 0) {
            // Create a safe buffer for assignment
            char *temp_buf = (char *)malloc(assign_len + 1);
            if (temp_buf) {
                memcpy(temp_buf, string_data, assign_len);
                temp_buf[assign_len] = '\0';
                
                // Test assignn function
                bool assign1_success = rosidl_runtime_c__String__assignn(
                    &single_str1, temp_buf, assign_len);
                bool assign2_success = rosidl_runtime_c__String__assignn(
                    &single_str2, temp_buf, assign_len);
                
                (void)assign1_success;
                (void)assign2_success;
                
                // Compare the assigned strings
                bool single_strings_equal = rosidl_runtime_c__String__are_equal(
                    &single_str1, &single_str2);
                (void)single_strings_equal;
                
                free(temp_buf);
            }
        }
        
        // Clean up individual strings
        rosidl_runtime_c__String__fini(&single_str1);
        rosidl_runtime_c__String__fini(&single_str2);
    }
    
    // Test 4: Modify one sequence slightly and compare again
    if (seq1->size > 0 && string_data_size > 0) {
        // Create a modified version of the first string in seq1
        size_t mod_len = (seq1->data[0].size > 0) ? seq1->data[0].size : 1;
        char *modified = (char *)malloc(mod_len + 1);
        if (modified) {
            if (seq1->data[0].size > 0) {
                memcpy(modified, seq1->data[0].data, mod_len);
                // Modify one character if possible
                if (mod_len > 0) {
                    modified[0] = (modified[0] == 'A') ? 'B' : 'A';
                }
            } else {
                modified[0] = 'X';
                modified[1] = '\0';
            }
            
            // Assign the modified string
            rosidl_runtime_c__String__assignn(&seq1->data[0], modified, mod_len);
            
            // Compare sequences again (should now be different)
            bool sequences_different = !rosidl_runtime_c__String__Sequence__are_equal(seq1, seq2);
            (void)sequences_different;
            
            free(modified);
        }
    }
    
    // Clean up
    free_string_sequence(seq1);
    free_string_sequence(seq2);
    
    return 0;
}
