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

// Note: <sstream> is a C++ header, but we're writing C code.
// We'll use C string functions instead.

// Forward declarations for types used in the API
typedef struct rosidl_runtime_c__boolean__Sequence rosidl_runtime_c__boolean__Sequence;
typedef struct rosidl_runtime_c__U16String__Sequence rosidl_runtime_c__U16String__Sequence;

// We need to define the rcutils allocator structure since it's used in the API
typedef struct rcutils_allocator_s
{
  void * (*allocate)(size_t size, void * state);
  void (*deallocate)(void * pointer, void * state);
  void * (*reallocate)(void * pointer, size_t size, void * state);
  void * state;
} rcutils_allocator_t;

// Mock implementation of rcutils_get_default_allocator
// since we don't have the actual header
static rcutils_allocator_t rcutils_get_default_allocator(void)
{
  rcutils_allocator_t allocator = {
    .allocate = malloc,
    .deallocate = free,
    .reallocate = realloc,
    .state = NULL
  };
  return allocator;
}

// We need to define the U16String structure since it's used in the API
typedef struct rosidl_runtime_c__U16String
{
  uint16_t * data;
  size_t size;
  size_t capacity;
} rosidl_runtime_c__U16String;

// We need forward declarations for the string functions used in the API
bool rosidl_runtime_c__U16String__init(rosidl_runtime_c__U16String * str);
void rosidl_runtime_c__U16String__fini(rosidl_runtime_c__U16String * str);
bool rosidl_runtime_c__U16String__copy(
  const rosidl_runtime_c__U16String * input,
  rosidl_runtime_c__U16String * output);
bool rosidl_runtime_c__U16String__are_equal(
  const rosidl_runtime_c__U16String * lhs,
  const rosidl_runtime_c__U16String * rhs);

// Mock implementations of the U16String functions since we don't have the actual implementations
// These are simplified versions that should work for fuzzing
bool rosidl_runtime_c__U16String__init(rosidl_runtime_c__U16String * str)
{
  if (!str) {
    return false;
  }
  str->data = NULL;
  str->size = 0;
  str->capacity = 0;
  return true;
}

void rosidl_runtime_c__U16String__fini(rosidl_runtime_c__U16String * str)
{
  if (!str) {
    return;
  }
  if (str->data) {
    free(str->data);
    str->data = NULL;
  }
  str->size = 0;
  str->capacity = 0;
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
  
  if (input->size == 0) {
    return true;
  }
  
  // Allocate memory for the copy
  output->data = (uint16_t *)malloc(input->size * sizeof(uint16_t));
  if (!output->data) {
    return false;
  }
  
  // Copy the data
  memcpy(output->data, input->data, input->size * sizeof(uint16_t));
  output->size = input->size;
  output->capacity = input->size;
  
  return true;
}

bool rosidl_runtime_c__U16String__are_equal(
  const rosidl_runtime_c__U16String * lhs,
  const rosidl_runtime_c__U16String * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  if (lhs->size == 0) {
    return true;
  }
  return memcmp(lhs->data, rhs->data, lhs->size * sizeof(uint16_t)) == 0;
}

// We also need the boolean sequence structure
typedef struct rosidl_runtime_c__boolean__Sequence
{
  bool * data;
  size_t size;
  size_t capacity;
} rosidl_runtime_c__boolean__Sequence;

// And the function it calls
bool rosidl_runtime_c__boolean__Sequence__are_equal(
  const rosidl_runtime_c__boolean__Sequence * lhs,
  const rosidl_runtime_c__boolean__Sequence * rhs);

// Mock implementation
bool rosidl_runtime_c__boolean__Sequence__are_equal(
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

// Now we can implement the actual API functions from the source code
bool rosidl_runtime_c__bool__Sequence__are_equal(
  const rosidl_runtime_c__boolean__Sequence * lhs,
  const rosidl_runtime_c__boolean__Sequence * rhs)
{
  return rosidl_runtime_c__boolean__Sequence__are_equal(lhs, rhs);
}

void rosidl_runtime_c__U16String__Sequence__fini(
  rosidl_runtime_c__U16String__Sequence * sequence)
{
  if (!sequence) {
    return;
  }
  if (sequence->data) {
    // ensure that data and capacity values are consistent
    assert(sequence->capacity > 0);
    // finalize all sequence elements
    for (size_t i = 0; i < sequence->capacity; ++i) {
      rosidl_runtime_c__U16String__fini(&sequence->data[i]);
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

bool rosidl_runtime_c__U16String__Sequence__copy(
  const rosidl_runtime_c__U16String__Sequence * input,
  rosidl_runtime_c__U16String__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t size =
      input->size * sizeof(rosidl_runtime_c__U16String);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    rosidl_runtime_c__U16String * data =
      (rosidl_runtime_c__U16String *)allocator.reallocate(
      output->data, size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!rosidl_runtime_c__U16String__init(&output->data[i])) {
        // If initialization of any new items fails, roll back all
        // previously initialized items. Existing items in output
        // are to be left unmodified.
        for (; i-- > output->capacity; ) {
          rosidl_runtime_c__U16String__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!rosidl_runtime_c__U16String__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}

bool rosidl_runtime_c__U16String__Sequence__are_equal(
  const rosidl_runtime_c__U16String__Sequence * lhs,
  const rosidl_runtime_c__U16String__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!rosidl_runtime_c__U16String__are_equal(
        &(lhs->data[i]), &(rhs->data[i])))
    {
      return false;
    }
  }
  return true;
}

bool rosidl_runtime_c__U16String__Sequence__init(
  rosidl_runtime_c__U16String__Sequence * sequence, size_t size)
{
  // RCUTILS_CAN_RETURN_WITH_ERROR_OF(false); - This is a macro we can't implement

  if (!sequence) {
    return false;
  }
  rosidl_runtime_c__U16String * data = NULL;
  if (size) {
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    data = (rosidl_runtime_c__U16String *)allocator.allocate(
      size * sizeof(rosidl_runtime_c__U16String), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all sequence elements
    for (size_t i = 0; i < size; ++i) {
      if (!rosidl_runtime_c__U16String__init(&data[i])) {
        /* free currently allocated and return false */
        for (; i-- > 0; ) {
          rosidl_runtime_c__U16String__fini(&data[i]);
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

// The U16String sequence structure
typedef struct rosidl_runtime_c__U16String__Sequence
{
  rosidl_runtime_c__U16String * data;
  size_t size;
  size_t capacity;
} rosidl_runtime_c__U16String__Sequence;

// Main fuzzer function
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  // Early exit if we don't have enough data
  if (size < 2) {
    return 0;
  }

  // Use the first byte to determine the number of elements in our sequences
  // Limit to a reasonable size to avoid excessive memory usage
  size_t num_elements = (data[0] % 16) + 1;  // 1-16 elements
  
  // Use the second byte to determine string lengths
  size_t str_len = (data[1] % 32) + 1;  // 1-32 characters
  
  // Initialize two boolean sequences for testing
  rosidl_runtime_c__boolean__Sequence bool_seq1 = {0};
  rosidl_runtime_c__boolean__Sequence bool_seq2 = {0};
  
  // Allocate memory for boolean sequences
  if (num_elements > 0) {
    bool_seq1.data = (bool *)malloc(num_elements * sizeof(bool));
    bool_seq2.data = (bool *)malloc(num_elements * sizeof(bool));
    
    if (!bool_seq1.data || !bool_seq2.data) {
      free(bool_seq1.data);
      free(bool_seq2.data);
      return 0;
    }
    
    bool_seq1.size = num_elements;
    bool_seq1.capacity = num_elements;
    bool_seq2.size = num_elements;
    bool_seq2.capacity = num_elements;
    
    // Initialize boolean sequences with data from fuzz input
    for (size_t i = 0; i < num_elements; i++) {
      size_t data_idx = (i + 2) % size;  // Wrap around if needed
      bool_seq1.data[i] = (data[data_idx] & 0x01) != 0;
      bool_seq2.data[i] = (data[data_idx] & 0x02) != 0;
    }
    
    // Test rosidl_runtime_c__bool__Sequence__are_equal
    bool bool_equal = rosidl_runtime_c__bool__Sequence__are_equal(&bool_seq1, &bool_seq2);
    (void)bool_equal;  // Use result to avoid unused variable warning
    
    // Clean up boolean sequences
    free(bool_seq1.data);
    free(bool_seq2.data);
  }
  
  // Initialize U16String sequences
  rosidl_runtime_c__U16String__Sequence u16_seq1 = {0};
  rosidl_runtime_c__U16String__Sequence u16_seq2 = {0};
  rosidl_runtime_c__U16String__Sequence u16_seq_copy = {0};
  
  // Test rosidl_runtime_c__U16String__Sequence__init
  bool init_success1 = rosidl_runtime_c__U16String__Sequence__init(&u16_seq1, num_elements);
  bool init_success2 = rosidl_runtime_c__U16String__Sequence__init(&u16_seq2, num_elements);
  
  if (!init_success1 || !init_success2) {
    // Clean up any partially initialized sequences
    rosidl_runtime_c__U16String__Sequence__fini(&u16_seq1);
    rosidl_runtime_c__U16String__Sequence__fini(&u16_seq2);
    return 0;
  }
  
  // Initialize U16String elements with data from fuzz input
  for (size_t i = 0; i < num_elements; i++) {
    // Calculate a safe length for this string
    size_t actual_str_len = str_len;
    if (actual_str_len * sizeof(uint16_t) > size - 2) {
      actual_str_len = (size - 2) / sizeof(uint16_t);
      if (actual_str_len == 0) {
        actual_str_len = 1;
      }
    }
    
    // Allocate memory for string data
    u16_seq1.data[i].data = (uint16_t *)malloc(actual_str_len * sizeof(uint16_t));
    u16_seq2.data[i].data = (uint16_t *)malloc(actual_str_len * sizeof(uint16_t));
    
    if (u16_seq1.data[i].data && u16_seq2.data[i].data) {
      u16_seq1.data[i].size = actual_str_len;
      u16_seq1.data[i].capacity = actual_str_len;
      u16_seq2.data[i].size = actual_str_len;
      u16_seq2.data[i].capacity = actual_str_len;
      
      // Fill with data from fuzz input
      for (size_t j = 0; j < actual_str_len; j++) {
        size_t data_idx = (i * actual_str_len + j + 2) % size;
        uint16_t val = data[data_idx];
        if (j % 2 == 1 && data_idx + 1 < size) {
          val = (val << 8) | data[data_idx + 1];
        }
        u16_seq1.data[i].data[j] = val;
        u16_seq2.data[i].data[j] = val ^ 0x00FF;  // Make them slightly different
      }
    } else {
      // Clean up on allocation failure
      free(u16_seq1.data[i].data);
      free(u16_seq2.data[i].data);
      u16_seq1.data[i].data = NULL;
      u16_seq2.data[i].data = NULL;
    }
  }
  
  // Test rosidl_runtime_c__U16String__Sequence__are_equal
  bool u16_equal = rosidl_runtime_c__U16String__Sequence__are_equal(&u16_seq1, &u16_seq2);
  (void)u16_equal;  // Use result to avoid unused variable warning
  
  // Test rosidl_runtime_c__U16String__Sequence__copy
  bool copy_success = rosidl_runtime_c__U16String__Sequence__copy(&u16_seq1, &u16_seq_copy);
  (void)copy_success;  // Use result to avoid unused variable warning
  
  // Test rosidl_runtime_c__U16String__Sequence__are_equal again with the copy
  if (copy_success) {
    bool copy_equal = rosidl_runtime_c__U16String__Sequence__are_equal(&u16_seq1, &u16_seq_copy);
    (void)copy_equal;  // Use result to avoid unused variable warning
  }
  
  // Clean up all U16String sequences
  rosidl_runtime_c__U16String__Sequence__fini(&u16_seq1);
  rosidl_runtime_c__U16String__Sequence__fini(&u16_seq2);
  rosidl_runtime_c__U16String__Sequence__fini(&u16_seq_copy);
  
  return 0;
}
