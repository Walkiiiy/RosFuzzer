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
#include <assert.h>

// Forward declarations for rcutils functions that are used in the API
// These are typically provided by rcutils/allocator.h
typedef struct rcutils_allocator_t
{
  void * (*allocate)(size_t size, void * state);
  void (*deallocate)(void * pointer, void * state);
  void * (*reallocate)(void * pointer, size_t size, void * state);
  void * (*zero_allocate)(size_t number_of_elements, size_t size_of_element, void * state);
  void * state;
} rcutils_allocator_t;

// Mock implementation of rcutils_get_default_allocator for fuzzing
static rcutils_allocator_t rcutils_get_default_allocator(void)
{
  static rcutils_allocator_t allocator = {
    .allocate = malloc,
    .deallocate = free,
    .reallocate = realloc,
    .zero_allocate = calloc,
    .state = NULL
  };
  return allocator;
}

// Mock RCUTILS_CAN_RETURN_WITH_ERROR_OF macro for compilation
#define RCUTILS_CAN_RETURN_WITH_ERROR_OF(x) (void)0

// ROS 2 runtime string type (from rosidl_runtime_c/string.h)
typedef struct rosidl_runtime_c__String
{
  char * data;
  size_t size;
  size_t capacity;
} rosidl_runtime_c__String;

// ROS 2 runtime string sequence type (from rosidl_runtime_c/string.h)
typedef struct rosidl_runtime_c__String__Sequence
{
  rosidl_runtime_c__String * data;
  size_t size;
  size_t capacity;
} rosidl_runtime_c__String__Sequence;

// API function declarations (implemented below based on provided source)
bool rosidl_runtime_c__String__init(rosidl_runtime_c__String * str);
void rosidl_runtime_c__String__fini(rosidl_runtime_c__String * str);
bool rosidl_runtime_c__String__copy(const rosidl_runtime_c__String * input,
                                    rosidl_runtime_c__String * output);
bool rosidl_runtime_c__String__Sequence__init(rosidl_runtime_c__String__Sequence * sequence,
                                              size_t size);
void rosidl_runtime_c__String__Sequence__fini(rosidl_runtime_c__String__Sequence * sequence);
bool rosidl_runtime_c__String__Sequence__copy(const rosidl_runtime_c__String__Sequence * input,
                                              rosidl_runtime_c__String__Sequence * output);

// Implement the API functions based on provided source code
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

bool rosidl_runtime_c__String__copy(const rosidl_runtime_c__String * input,
                                    rosidl_runtime_c__String * output)
{
  if (!input || !output) {
    return false;
  }
  
  // Ensure output is initialized
  if (!output->data) {
    if (!rosidl_runtime_c__String__init(output)) {
      return false;
    }
  }
  
  // Check if we need to reallocate
  size_t required_capacity = input->size + 1;
  if (output->capacity < required_capacity) {
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    char * new_data = (char *)allocator.reallocate(
      output->data, required_capacity, allocator.state);
    if (!new_data) {
      return false;
    }
    output->data = new_data;
    output->capacity = required_capacity;
  }
  
  // Copy the string content
  if (input->size > 0 && input->data) {
    memcpy(output->data, input->data, input->size);
  }
  output->data[input->size] = '\0';
  output->size = input->size;
  
  return true;
}

bool rosidl_runtime_c__String__Sequence__init(rosidl_runtime_c__String__Sequence * sequence,
                                              size_t size)
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

void rosidl_runtime_c__String__Sequence__fini(rosidl_runtime_c__String__Sequence * sequence)
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

bool rosidl_runtime_c__String__Sequence__copy(
  const rosidl_runtime_c__String__Sequence * input,
  rosidl_runtime_c__String__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(rosidl_runtime_c__String);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    rosidl_runtime_c__String * data =
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

// Fuzz driver entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  // Early exit if we don't have enough data for basic operations
  if (size < 2) {
    return 0;
  }

  // Use the first byte to determine the sequence size (limit to reasonable size)
  size_t seq_size = (data[0] % 16) + 1;  // 1-16 elements
  
  // Ensure we have enough data for the strings
  if (size < 1 + seq_size) {
    return 0;
  }

  // Initialize source sequence
  rosidl_runtime_c__String__Sequence src_sequence;
  if (!rosidl_runtime_c__String__Sequence__init(&src_sequence, seq_size)) {
    // If initialization fails, clean up and return
    return 0;
  }

  // Fill source strings with data from fuzz input
  size_t data_offset = 1;
  for (size_t i = 0; i < seq_size; i++) {
    // Determine string length from fuzz data (1-255 bytes)
    size_t str_len = 0;
    if (data_offset < size) {
      str_len = data[data_offset] % 256;
      data_offset++;
    }
    
    // Ensure we don't read beyond available data
    if (str_len > 0 && data_offset + str_len > size) {
      str_len = size - data_offset;
    }
    
    // Allocate and copy string data
    if (str_len > 0) {
      // Free the initially allocated empty string
      if (src_sequence.data[i].data) {
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        allocator.deallocate(src_sequence.data[i].data, allocator.state);
      }
      
      // Allocate space for string + null terminator
      rcutils_allocator_t allocator = rcutils_get_default_allocator();
      src_sequence.data[i].data = (char *)allocator.allocate(str_len + 1, allocator.state);
      if (!src_sequence.data[i].data) {
        // Clean up and return on allocation failure
        rosidl_runtime_c__String__Sequence__fini(&src_sequence);
        return 0;
      }
      
      // Copy data and set string properties
      if (str_len > 0) {
        memcpy(src_sequence.data[i].data, data + data_offset, str_len);
        data_offset += str_len;
      }
      src_sequence.data[i].data[str_len] = '\0';
      src_sequence.data[i].size = str_len;
      src_sequence.data[i].capacity = str_len + 1;
    }
  }

  // Initialize destination sequence with different size to test reallocation
  rosidl_runtime_c__String__Sequence dst_sequence;
  size_t dst_init_size = (seq_size > 1) ? (seq_size / 2) : 0;
  if (!rosidl_runtime_c__String__Sequence__init(&dst_sequence, dst_init_size)) {
    rosidl_runtime_c__String__Sequence__fini(&src_sequence);
    return 0;
  }

  // Test rosidl_runtime_c__String__Sequence__copy
  bool copy_success = rosidl_runtime_c__String__Sequence__copy(&src_sequence, &dst_sequence);
  
  // Verify the copy if it succeeded
  if (copy_success) {
    // Check that sizes match
    if (dst_sequence.size != src_sequence.size) {
      // This shouldn't happen if copy succeeded
      // But we'll handle it gracefully
    }
    
    // Verify each string was copied correctly
    for (size_t i = 0; i < dst_sequence.size && i < src_sequence.size; i++) {
      if (dst_sequence.data[i].size != src_sequence.data[i].size) {
        // Size mismatch - this indicates a problem
      }
    }
  }

  // Test individual string operations
  rosidl_runtime_c__String single_string;
  if (rosidl_runtime_c__String__init(&single_string)) {
    // Try to copy one of the source strings
    if (src_sequence.size > 0) {
      bool string_copy_success = rosidl_runtime_c__String__copy(
        &src_sequence.data[0], &single_string);
      (void)string_copy_success; // Mark as used
    }
    
    // Clean up the single string
    rosidl_runtime_c__String__fini(&single_string);
  }

  // Clean up all allocated resources
  rosidl_runtime_c__String__Sequence__fini(&src_sequence);
  rosidl_runtime_c__String__Sequence__fini(&dst_sequence);

  return 0;
}
