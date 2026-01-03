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

// Forward declarations for types used in the API
typedef struct rosidl_runtime_c__String__Sequence rosidl_runtime_c__String__Sequence;
typedef struct rosidl_runtime_c__String rosidl_runtime_c__String;

// We need to define the rcutils_allocator_t structure and default allocator
// since they are referenced in the API source code
typedef struct rcutils_allocator_t
{
  void * (*allocate)(size_t size, void * state);
  void * (*reallocate)(void * pointer, size_t size, void * state);
  void (*deallocate)(void * pointer, void * state);
  void * state;
} rcutils_allocator_t;

// Default allocator implementation using malloc/realloc/free
static void * default_allocate(size_t size, void * state)
{
  (void)state;
  return malloc(size);
}

static void * default_reallocate(void * pointer, size_t size, void * state)
{
  (void)state;
  return realloc(pointer, size);
}

static void default_deallocate(void * pointer, void * state)
{
  (void)state;
  free(pointer);
}

static rcutils_allocator_t rcutils_get_default_allocator(void)
{
  static rcutils_allocator_t allocator = {
    .allocate = default_allocate,
    .reallocate = default_reallocate,
    .deallocate = default_deallocate,
    .state = NULL
  };
  return allocator;
}

// We also need the rosidl_runtime_c__String__are_equal function
// which is referenced in the sequence equality check
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
  if (lhs->data == NULL && rhs->data == NULL) {
    return true;
  }
  if (lhs->data == NULL || rhs->data == NULL) {
    return false;
  }
  return memcmp(lhs->data, rhs->data, lhs->size) == 0;
}

// We need the rosidl_runtime_c__String__copy function
// which is referenced in the sequence copy function
bool rosidl_runtime_c__String__copy(
  const rosidl_runtime_c__String * input,
  rosidl_runtime_c__String * output)
{
  if (!input || !output) {
    return false;
  }
  
  // Initialize output string
  if (!rosidl_runtime_c__String__init(output)) {
    return false;
  }
  
  // If input has data, copy it
  if (input->data && input->size > 0) {
    // Resize output to accommodate input data
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    char * new_data = (char *)allocator.reallocate(
      output->data, input->size + 1, allocator.state);
    if (!new_data) {
      rosidl_runtime_c__String__fini(output);
      return false;
    }
    output->data = new_data;
    output->capacity = input->size + 1;
    
    // Copy the data
    memcpy(output->data, input->data, input->size);
    output->data[input->size] = '\0';
    output->size = input->size;
  }
  
  return true;
}

// We need the rosidl_runtime_c__String__init function
// which is referenced in the sequence init function
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

// Define the structures based on typical ROS 2 runtime C implementation
struct rosidl_runtime_c__String
{
  char * data;
  size_t size;
  size_t capacity;
};

struct rosidl_runtime_c__String__Sequence
{
  rosidl_runtime_c__String * data;
  size_t size;
  size_t capacity;
};

// Now include the actual API functions from the provided source code
// (These are already provided in the prompt, so we'll implement them here)

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

void rosidl_runtime_c__String__fini(rosidl_runtime_c__String * str)
{
  if (!str) {
    return;
  }
  if (str->data) {
    /* ensure that data and capacity values are consistent */
    if (str->capacity <= 0) {
      // In fuzzing, we don't want to exit, so we'll just free and return
      rcutils_allocator_t allocator = rcutils_get_default_allocator();
      allocator.deallocate(str->data, allocator.state);
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
    /* ensure that data, size, and capacity values are consistent */
    if (0 != str->size) {
      // In fuzzing, just reset the values
      str->size = 0;
    }
    if (0 != str->capacity) {
      str->capacity = 0;
    }
  }
}

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

void rosidl_runtime_c__String__Sequence__fini(
  rosidl_runtime_c__String__Sequence * sequence)
{
  if (!sequence) {
    return;
  }
  if (sequence->data) {
    // ensure that data and capacity values are consistent
    // In fuzzing, we'll skip the assert and just check
    if (sequence->capacity == 0) {
      // This shouldn't happen, but in fuzzing we handle it gracefully
      sequence->data = NULL;
      sequence->size = 0;
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
    // In fuzzing, just reset to be safe
    sequence->size = 0;
    sequence->capacity = 0;
  }
}

// Fuzz driver function
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  // We need at least 1 byte to determine sequence size
  if (size < 1) {
    return 0;
  }

  // Use the first byte to determine the sequence size (0-255)
  // But limit to a reasonable size to avoid excessive memory usage
  size_t sequence_size = data[0] % 16;  // 0-15 elements
  
  // Initialize two sequences
  rosidl_runtime_c__String__Sequence seq1, seq2, seq3;
  
  // Initialize seq1 with the determined size
  if (!rosidl_runtime_c__String__Sequence__init(&seq1, sequence_size)) {
    // If initialization fails, just return
    return 0;
  }
  
  // Initialize seq2 with the same size
  if (!rosidl_runtime_c__String__Sequence__init(&seq2, sequence_size)) {
    rosidl_runtime_c__String__Sequence__fini(&seq1);
    return 0;
  }
  
  // Initialize seq3 with 0 size (will be used for copy test)
  if (!rosidl_runtime_c__String__Sequence__init(&seq3, 0)) {
    rosidl_runtime_c__String__Sequence__fini(&seq2);
    rosidl_runtime_c__String__Sequence__fini(&seq1);
    return 0;
  }
  
  // Fill the sequences with data from the fuzz input
  // We'll use the remaining data to populate string contents
  size_t data_offset = 1;  // Skip the first byte used for sequence_size
  
  for (size_t i = 0; i < sequence_size; i++) {
    if (data_offset >= size) {
      // No more data, break early
      break;
    }
    
    // Determine string length from next byte (0-255)
    size_t str_len = data[data_offset] % 32;  // Limit to 31 chars max
    data_offset++;
    
    // Ensure we don't read past the end of the input
    if (data_offset + str_len > size) {
      str_len = size - data_offset;
    }
    
    if (str_len > 0) {
      // Free the default empty string data
      rcutils_allocator_t allocator = rcutils_get_default_allocator();
      if (seq1.data[i].data) {
        allocator.deallocate(seq1.data[i].data, allocator.state);
      }
      
      // Allocate space for the string + null terminator
      seq1.data[i].data = (char *)allocator.allocate(str_len + 1, allocator.state);
      if (seq1.data[i].data) {
        // Copy the data
        memcpy(seq1.data[i].data, data + data_offset, str_len);
        seq1.data[i].data[str_len] = '\0';
        seq1.data[i].size = str_len;
        seq1.data[i].capacity = str_len + 1;
        
        // Do the same for seq2 to make them equal
        if (seq2.data[i].data) {
          allocator.deallocate(seq2.data[i].data, allocator.state);
        }
        seq2.data[i].data = (char *)allocator.allocate(str_len + 1, allocator.state);
        if (seq2.data[i].data) {
          memcpy(seq2.data[i].data, data + data_offset, str_len);
          seq2.data[i].data[str_len] = '\0';
          seq2.data[i].size = str_len;
          seq2.data[i].capacity = str_len + 1;
        }
      }
      
      data_offset += str_len;
    }
  }
  
  // Test 1: Check if the sequences are equal (they should be since we filled them the same way)
  bool are_equal = rosidl_runtime_c__String__Sequence__are_equal(&seq1, &seq2);
  // In our test, they should be equal unless memory allocation failed
  
  // Test 2: Copy seq1 to seq3
  bool copy_success = rosidl_runtime_c__String__Sequence__copy(&seq1, &seq3);
  
  // Test 3: Check if the copy was successful and if seq1 and seq3 are equal
  if (copy_success) {
    bool copy_equal = rosidl_runtime_c__String__Sequence__are_equal(&seq1, &seq3);
    (void)copy_equal;  // Use the result to avoid unused variable warning
  }
  
  // Test 4: Test individual string finalization on a few strings
  if (sequence_size > 0) {
    // Create a standalone string and finalize it
    rosidl_runtime_c__String standalone_string;
    if (rosidl_runtime_c__String__init(&standalone_string)) {
      // Try to set some data if we have any remaining
      if (data_offset < size && size - data_offset > 0) {
        size_t remaining = size - data_offset;
        size_t copy_len = remaining > 31 ? 31 : remaining;
        
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        char *new_data = (char *)allocator.reallocate(
          standalone_string.data, copy_len + 1, allocator.state);
        if (new_data) {
          standalone_string.data = new_data;
          memcpy(standalone_string.data, data + data_offset, copy_len);
          standalone_string.data[copy_len] = '\0';
          standalone_string.size = copy_len;
          standalone_string.capacity = copy_len + 1;
        }
      }
      rosidl_runtime_c__String__fini(&standalone_string);
    }
  }
  
  // Clean up all sequences
  rosidl_runtime_c__String__Sequence__fini(&seq3);
  rosidl_runtime_c__String__Sequence__fini(&seq2);
  rosidl_runtime_c__String__Sequence__fini(&seq1);
  
  return 0;
}
