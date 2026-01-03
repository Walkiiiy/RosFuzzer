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
// We'll use C standard library functions instead.

// Forward declarations for rcutils functions that are used in the API
typedef struct rcutils_allocator_t
{
  void *(*allocate)(size_t size, void *state);
  void (*deallocate)(void *pointer, void *state);
  void *(*reallocate)(void *pointer, size_t size, void *state);
  void *state;
} rcutils_allocator_t;

// Mock implementation of rcutils_get_default_allocator for fuzzing
static rcutils_allocator_t rcutils_get_default_allocator(void)
{
  static rcutils_allocator_t allocator = {
    .allocate = malloc,
    .deallocate = free,
    .reallocate = realloc,
    .state = NULL
  };
  return allocator;
}

// Mock implementation of RCUTILS_CAN_RETURN_WITH_ERROR_OF macro
#define RCUTILS_CAN_RETURN_WITH_ERROR_OF(x) (void)0

// Type definitions based on the API source code
typedef struct rosidl_runtime_c__U16String
{
  uint16_t *data;
  size_t size;
  size_t capacity;
} rosidl_runtime_c__U16String;

typedef struct rosidl_runtime_c__U16String__Sequence
{
  rosidl_runtime_c__U16String *data;
  size_t size;
  size_t capacity;
} rosidl_runtime_c__U16String__Sequence;

// Function prototypes from the API
bool rosidl_runtime_c__U16String__init(rosidl_runtime_c__U16String *str);
void rosidl_runtime_c__U16String__fini(rosidl_runtime_c__U16String *str);
bool rosidl_runtime_c__U16String__copy(
  const rosidl_runtime_c__U16String *input,
  rosidl_runtime_c__U16String *output);

// Mock implementations of the missing U16String functions
bool rosidl_runtime_c__U16String__init(rosidl_runtime_c__U16String *str)
{
  if (!str) {
    return false;
  }
  str->data = NULL;
  str->size = 0;
  str->capacity = 0;
  return true;
}

void rosidl_runtime_c__U16String__fini(rosidl_runtime_c__U16String *str)
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
  const rosidl_runtime_c__U16String *input,
  rosidl_runtime_c__U16String *output)
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
    rosidl_runtime_c__U16String__fini(output);
    return false;
  }
  
  // Copy the data
  memcpy(output->data, input->data, input->size * sizeof(uint16_t));
  output->size = input->size;
  output->capacity = input->size;
  
  return true;
}

// The actual API functions from the provided source code
void rosidl_runtime_c__U16String__Sequence__fini(
  rosidl_runtime_c__U16String__Sequence * sequence);

bool rosidl_runtime_c__U16String__Sequence__copy(
  const rosidl_runtime_c__U16String__Sequence * input,
  rosidl_runtime_c__U16String__Sequence * output);

bool rosidl_runtime_c__U16String__Sequence__init(
  rosidl_runtime_c__U16String__Sequence * sequence, size_t size);

bool rosidl_runtime_c__U16String__Sequence__are_equal(
  const rosidl_runtime_c__U16String__Sequence * lhs,
  const rosidl_runtime_c__U16String__Sequence * rhs);

bool rosidl_runtime_c__U16String__are_equal(
  const rosidl_runtime_c__U16String * lhs,
  const rosidl_runtime_c__U16String * rhs);

// Implementations of the API functions (copied from provided source code)
void
rosidl_runtime_c__U16String__Sequence__fini(
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

bool
rosidl_runtime_c__U16String__Sequence__copy(
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

bool
rosidl_runtime_c__U16String__Sequence__init(
  rosidl_runtime_c__U16String__Sequence * sequence, size_t size)
{
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(false);

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

bool
rosidl_runtime_c__U16String__Sequence__are_equal(
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

bool
rosidl_runtime_c__U16String__are_equal(
  const rosidl_runtime_c__U16String * lhs,
  const rosidl_runtime_c__U16String * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  return memcmp(lhs->data, rhs->data, lhs->size * sizeof(uint16_t)) == 0;
}

// Helper function to create a U16String from fuzz data
static bool create_u16string_from_fuzz_data(
  rosidl_runtime_c__U16String *str,
  const uint8_t *data,
  size_t size,
  size_t offset)
{
  if (!str || offset >= size) {
    return false;
  }
  
  // Calculate how many uint16_t we can create from remaining data
  size_t remaining = size - offset;
  size_t u16_count = remaining / sizeof(uint16_t);
  
  if (u16_count == 0) {
    // Initialize empty string
    return rosidl_runtime_c__U16String__init(str);
  }
  
  // Initialize the string
  if (!rosidl_runtime_c__U16String__init(str)) {
    return false;
  }
  
  // Allocate memory for the string data
  str->data = (uint16_t *)malloc(u16_count * sizeof(uint16_t));
  if (!str->data) {
    rosidl_runtime_c__U16String__fini(str);
    return false;
  }
  
  // Copy data from fuzz input
  memcpy(str->data, data + offset, u16_count * sizeof(uint16_t));
  str->size = u16_count;
  str->capacity = u16_count;
  
  return true;
}

// Fuzz driver entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  // Early exit for insufficient data
  if (size < sizeof(uint16_t) * 2) {
    return 0;
  }
  
  // Initialize variables
  rosidl_runtime_c__U16String__Sequence seq1 = {0};
  rosidl_runtime_c__U16String__Sequence seq2 = {0};
  rosidl_runtime_c__U16String__Sequence seq3 = {0};
  
  // Determine sequence size from fuzz data (limit to reasonable size)
  size_t seq_size = (data[0] % 8) + 1;  // 1-8 elements
  
  // 1. Initialize first sequence
  if (!rosidl_runtime_c__U16String__Sequence__init(&seq1, seq_size)) {
    goto cleanup;
  }
  
  // Fill sequence with data from fuzz input
  size_t data_offset = 1;
  for (size_t i = 0; i < seq_size && data_offset < size; i++) {
    // Create a U16String from fuzz data
    if (!create_u16string_from_fuzz_data(&seq1.data[i], data, size, data_offset)) {
      // If creation fails, skip this element but continue with others
      seq1.data[i].size = 0;
    } else {
      // Advance offset by the amount of data used
      size_t used_bytes = seq1.data[i].size * sizeof(uint16_t);
      data_offset += used_bytes;
      
      // Ensure we don't overflow
      if (data_offset > size) {
        data_offset = size;
      }
    }
  }
  
  // 2. Copy sequence to second sequence
  if (!rosidl_runtime_c__U16String__Sequence__copy(&seq1, &seq2)) {
    goto cleanup;
  }
  
  // 3. Initialize third sequence with different size
  size_t seq3_size = (seq_size > 1) ? (seq_size - 1) : 1;
  if (!rosidl_runtime_c__U16String__Sequence__init(&seq3, seq3_size)) {
    goto cleanup;
  }
  
  // Fill third sequence with data (reusing fuzz data from beginning)
  data_offset = size > 10 ? size - 10 : 0;  // Use different part of fuzz data
  for (size_t i = 0; i < seq3_size && data_offset < size; i++) {
    if (!create_u16string_from_fuzz_data(&seq3.data[i], data, size, data_offset)) {
      seq3.data[i].size = 0;
    } else {
      size_t used_bytes = seq3.data[i].size * sizeof(uint16_t);
      data_offset += used_bytes;
      if (data_offset > size) {
        data_offset = size;
      }
    }
  }
  
  // 4. Compare sequences using are_equal
  // This will likely return false since sequences have different sizes or content
  bool equal1 = rosidl_runtime_c__U16String__Sequence__are_equal(&seq1, &seq2);
  bool equal2 = rosidl_runtime_c__U16String__Sequence__are_equal(&seq1, &seq3);
  
  // 5. Compare individual strings using U16String__are_equal
  if (seq1.size > 0 && seq2.size > 0) {
    bool string_equal = rosidl_runtime_c__U16String__are_equal(
      &seq1.data[0], &seq2.data[0]);
    
    // Also compare with itself (should be true)
    bool self_equal = rosidl_runtime_c__U16String__are_equal(
      &seq1.data[0], &seq1.data[0]);
    
    (void)string_equal;  // Mark as used
    (void)self_equal;    // Mark as used
  }
  
  // Mark comparison results as used to avoid compiler warnings
  (void)equal1;
  (void)equal2;

cleanup:
  // 6. Finalize all sequences (clean up memory)
  rosidl_runtime_c__U16String__Sequence__fini(&seq1);
  rosidl_runtime_c__U16String__Sequence__fini(&seq2);
  rosidl_runtime_c__U16String__Sequence__fini(&seq3);
  
  return 0;
}
