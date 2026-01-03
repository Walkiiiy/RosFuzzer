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

// Forward declarations for functions we need but don't have source for
// Based on API usage patterns
typedef struct rcutils_allocator_s
{
  void * (*allocate)(size_t size, void * state);
  void (*deallocate)(void * pointer, void * state);
  void * (*reallocate)(void * pointer, size_t size, void * state);
  void * state;
} rcutils_allocator_t;

rcutils_allocator_t rcutils_get_default_allocator(void);

bool rosidl_runtime_c__U16String__init(rosidl_runtime_c__U16String * str);
bool rosidl_runtime_c__U16String__copy(
  const rosidl_runtime_c__U16String * input,
  rosidl_runtime_c__U16String * output);

// Mock implementations for missing functions
rcutils_allocator_t rcutils_get_default_allocator(void) {
  static rcutils_allocator_t allocator = {
    .allocate = malloc,
    .deallocate = free,
    .reallocate = realloc,
    .state = NULL
  };
  return allocator;
}

bool rosidl_runtime_c__U16String__init(rosidl_runtime_c__U16String * str) {
  if (!str) {
    return false;
  }
  str->data = NULL;
  str->size = 0;
  str->capacity = 0;
  return true;
}

bool rosidl_runtime_c__U16String__copy(
  const rosidl_runtime_c__U16String * input,
  rosidl_runtime_c__U16String * output)
{
  if (!input || !output) {
    return false;
  }
  
  // Ensure output is initialized
  if (!rosidl_runtime_c__U16String__init(output)) {
    return false;
  }
  
  // Allocate memory if needed
  if (output->capacity < input->size) {
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    uint16_t * data = (uint16_t *)allocator.reallocate(
      output->data, input->size * sizeof(uint16_t), allocator.state);
    if (!data && input->size > 0) {
      return false;
    }
    output->data = data;
    output->capacity = input->size;
  }
  
  // Copy data
  output->size = input->size;
  if (input->size > 0 && input->data) {
    memcpy(output->data, input->data, input->size * sizeof(uint16_t));
  }
  return true;
}

// Helper function to create boolean sequence from fuzz data
static bool create_bool_sequence_from_fuzz(
    const uint8_t *data, 
    size_t size, 
    rosidl_runtime_c__boolean__Sequence *seq)
{
    if (!seq) {
        return false;
    }
    
    // Initialize sequence
    seq->data = NULL;
    seq->size = 0;
    seq->capacity = 0;
    
    if (size == 0) {
        return true;
    }
    
    // Use fuzz data to determine sequence size (limited to prevent excessive allocation)
    size_t seq_size = data[0] % 256;  // Limit to reasonable size
    if (seq_size > size) {
        seq_size = size % 256;
    }
    
    if (seq_size == 0) {
        return true;
    }
    
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    bool *bool_data = (bool *)allocator.allocate(
        seq_size * sizeof(bool), allocator.state);
    
    if (!bool_data) {
        return false;
    }
    
    // Fill with fuzz data
    for (size_t i = 0; i < seq_size; i++) {
        bool_data[i] = (data[i % size] & 0x01) != 0;
    }
    
    seq->data = bool_data;
    seq->size = seq_size;
    seq->capacity = seq_size;
    
    return true;
}

// Helper function to create U16String from fuzz data
static bool create_u16string_from_fuzz(
    const uint8_t *data, 
    size_t size, 
    rosidl_runtime_c__U16String *str)
{
    if (!str) {
        return false;
    }
    
    if (!rosidl_runtime_c__U16String__init(str)) {
        return false;
    }
    
    if (size == 0) {
        return true;
    }
    
    // Convert bytes to uint16_t (UTF-16 code units)
    // Use half the size for UTF-16 to ensure we have pairs
    size_t u16_size = size / 2;
    if (u16_size == 0) {
        u16_size = 1;
    }
    
    // Limit size to prevent excessive allocation
    if (u16_size > 256) {
        u16_size = 256;
    }
    
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    uint16_t *u16_data = (uint16_t *)allocator.allocate(
        u16_size * sizeof(uint16_t), allocator.state);
    
    if (!u16_data) {
        rosidl_runtime_c__U16String__fini(str);
        return false;
    }
    
    // Fill with fuzz data
    for (size_t i = 0; i < u16_size; i++) {
        if (i * 2 + 1 < size) {
            u16_data[i] = (data[i * 2] << 8) | data[i * 2 + 1];
        } else {
            u16_data[i] = data[i % size];
        }
    }
    
    str->data = u16_data;
    str->size = u16_size;
    str->capacity = u16_size;
    
    return true;
}

// Helper function to create U16String sequence from fuzz data
static bool create_u16string_sequence_from_fuzz(
    const uint8_t *data, 
    size_t size, 
    rosidl_runtime_c__U16String__Sequence *seq)
{
    if (!seq) {
        return false;
    }
    
    // Initialize sequence
    seq->data = NULL;
    seq->size = 0;
    seq->capacity = 0;
    
    if (size == 0) {
        return true;
    }
    
    // Determine sequence size from fuzz data
    size_t seq_size = data[0] % 16;  // Limit to reasonable size
    if (seq_size == 0 && size > 1) {
        seq_size = 1;
    }
    
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    rosidl_runtime_c__U16String *str_data = 
        (rosidl_runtime_c__U16String *)allocator.allocate(
            seq_size * sizeof(rosidl_runtime_c__U16String), allocator.state);
    
    if (!str_data && seq_size > 0) {
        return false;
    }
    
    // Initialize all strings
    for (size_t i = 0; i < seq_size; i++) {
        if (!rosidl_runtime_c__U16String__init(&str_data[i])) {
            // Clean up already initialized strings
            for (size_t j = 0; j < i; j++) {
                rosidl_runtime_c__U16String__fini(&str_data[j]);
            }
            allocator.deallocate(str_data, allocator.state);
            return false;
        }
    }
    
    // Fill strings with fuzz data
    for (size_t i = 0; i < seq_size; i++) {
        // Use different portions of fuzz data for each string
        size_t offset = (i * 17) % size;  // Simple offset calculation
        const uint8_t *str_data_ptr = data + offset;
        size_t str_data_size = size - offset;
        if (str_data_size > 64) {
            str_data_size = 64;  // Limit string size
        }
        
        if (!create_u16string_from_fuzz(str_data_ptr, str_data_size, &str_data[i])) {
            // Clean up on failure
            for (size_t j = 0; j <= i; j++) {
                rosidl_runtime_c__U16String__fini(&str_data[j]);
            }
            allocator.deallocate(str_data, allocator.state);
            return false;
        }
    }
    
    seq->data = str_data;
    seq->size = seq_size;
    seq->capacity = seq_size;
    
    return true;
}

// Helper function to free boolean sequence
static void free_bool_sequence(rosidl_runtime_c__boolean__Sequence *seq) {
    if (seq && seq->data) {
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        allocator.deallocate(seq->data, allocator.state);
        seq->data = NULL;
        seq->size = 0;
        seq->capacity = 0;
    }
}

// Helper function to free U16String sequence
static void free_u16string_sequence(rosidl_runtime_c__U16String__Sequence *seq) {
    if (seq && seq->data) {
        for (size_t i = 0; i < seq->size; i++) {
            rosidl_runtime_c__U16String__fini(&seq->data[i]);
        }
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        allocator.deallocate(seq->data, allocator.state);
        seq->data = NULL;
        seq->size = 0;
        seq->capacity = 0;
    }
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Ensure we have some data to work with
    if (!data || size == 0) {
        return 0;
    }
    
    // 1. Test rosidl_runtime_c__bool__Sequence__are_equal
    rosidl_runtime_c__boolean__Sequence bool_seq1 = {0};
    rosidl_runtime_c__boolean__Sequence bool_seq2 = {0};
    
    if (!create_bool_sequence_from_fuzz(data, size, &bool_seq1)) {
        // Cleanup if initialization failed
        free_bool_sequence(&bool_seq1);
        free_bool_sequence(&bool_seq2);
        return 0;
    }
    
    // Create a second sequence that's identical
    if (!create_bool_sequence_from_fuzz(data, size, &bool_seq2)) {
        free_bool_sequence(&bool_seq1);
        free_bool_sequence(&bool_seq2);
        return 0;
    }
    
    // Test equality - should be true since we created them the same way
    bool bool_seqs_equal = rosidl_runtime_c__bool__Sequence__are_equal(&bool_seq1, &bool_seq2);
    
    // Clean up boolean sequences
    free_bool_sequence(&bool_seq1);
    free_bool_sequence(&bool_seq2);
    
    // 2. Test rosidl_runtime_c__U16String__fini and rosidl_runtime_c__U16String__are_equal
    rosidl_runtime_c__U16String u16str1 = {0};
    rosidl_runtime_c__U16String u16str2 = {0};
    
    if (!create_u16string_from_fuzz(data, size, &u16str1)) {
        rosidl_runtime_c__U16String__fini(&u16str1);
        rosidl_runtime_c__U16String__fini(&u16str2);
        return 0;
    }
    
    // Create a copy for comparison
    if (!rosidl_runtime_c__U16String__copy(&u16str1, &u16str2)) {
        rosidl_runtime_c__U16String__fini(&u16str1);
        rosidl_runtime_c__U16String__fini(&u16str2);
        return 0;
    }
    
    // Test equality - should be true
    bool u16strings_equal = rosidl_runtime_c__U16String__are_equal(&u16str1, &u16str2);
    
    // Clean up U16Strings
    rosidl_runtime_c__U16String__fini(&u16str1);
    rosidl_runtime_c__U16String__fini(&u16str2);
    
    // 3. Test rosidl_runtime_c__U16String__Sequence__copy and 
    //    rosidl_runtime_c__U16String__Sequence__are_equal
    rosidl_runtime_c__U16String__Sequence u16seq1 = {0};
    rosidl_runtime_c__U16String__Sequence u16seq2 = {0};
    
    if (!create_u16string_sequence_from_fuzz(data, size, &u16seq1)) {
        free_u16string_sequence(&u16seq1);
        free_u16string_sequence(&u16seq2);
        return 0;
    }
    
    // Initialize second sequence
    u16seq2.data = NULL;
    u16seq2.size = 0;
    u16seq2.capacity = 0;
    
    // Copy first sequence to second
    bool copy_success = rosidl_runtime_c__U16String__Sequence__copy(&u16seq1, &u16seq2);
    
    if (copy_success) {
        // Test equality - should be true after copy
        bool u16seqs_equal = rosidl_runtime_c__U16String__Sequence__are_equal(&u16seq1, &u16seq2);
        
        // Also test individual string equality within sequences
        if (u16seq1.size > 0 && u16seq2.size > 0) {
            for (size_t i = 0; i < u16seq1.size && i < u16seq2.size; i++) {
                bool individual_equal = rosidl_runtime_c__U16String__are_equal(
                    &u16seq1.data[i], &u16seq2.data[i]);
                // Individual strings should be equal after copy
                (void)individual_equal; // Use result to avoid unused variable warning
            }
        }
    }
    
    // Clean up U16String sequences
    free_u16string_sequence(&u16seq1);
    free_u16string_sequence(&u16seq2);
    
    // Use the results to avoid compiler warnings about unused variables
    (void)bool_seqs_equal;
    (void)u16strings_equal;
    (void)copy_success;
    
    return 0;
}
