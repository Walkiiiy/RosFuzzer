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

// Forward declarations for rcutils functions used in the API
typedef struct rcutils_allocator_t
{
  void *(*allocate)(size_t size, void *state);
  void (*deallocate)(void *pointer, void *state);
  void *(*reallocate)(void *pointer, size_t size, void *state);
  void *state;
} rcutils_allocator_t;

rcutils_allocator_t rcutils_get_default_allocator(void);

// U16String functions used in the API
bool rosidl_runtime_c__U16String__init(rosidl_runtime_c__U16String *str);
void rosidl_runtime_c__U16String__fini(rosidl_runtime_c__U16String *str);
bool rosidl_runtime_c__U16String__copy(
  const rosidl_runtime_c__U16String *input,
  rosidl_runtime_c__U16String *output);

// U16String sequence structure
typedef struct rosidl_runtime_c__U16String__Sequence
{
  rosidl_runtime_c__U16String *data;
  size_t size;
  size_t capacity;
} rosidl_runtime_c__U16String__Sequence;

// Function prototypes from the provided API
void rosidl_runtime_c__U16String__Sequence__fini(
  rosidl_runtime_c__U16String__Sequence *sequence);
bool rosidl_runtime_c__U16String__Sequence__copy(
  const rosidl_runtime_c__U16String__Sequence *input,
  rosidl_runtime_c__U16String__Sequence *output);
bool rosidl_runtime_c__U16String__Sequence__init(
  rosidl_runtime_c__U16String__Sequence *sequence, size_t size);
void rosidl_runtime_c__U16String__Sequence__destroy(
  rosidl_runtime_c__U16String__Sequence *sequence);
rosidl_runtime_c__U16String__Sequence *
rosidl_runtime_c__U16String__Sequence__create(size_t size);

// Mock implementations of rcutils functions for fuzzing
rcutils_allocator_t rcutils_get_default_allocator(void)
{
  static rcutils_allocator_t allocator = {
    .allocate = malloc,
    .deallocate = free,
    .reallocate = realloc,
    .state = NULL
  };
  return allocator;
}

// Mock implementations of U16String functions for fuzzing
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
  if (str && str->data) {
    free(str->data);
    str->data = NULL;
    str->size = 0;
    str->capacity = 0;
  }
}

bool rosidl_runtime_c__U16String__copy(
  const rosidl_runtime_c__U16String *input,
  rosidl_runtime_c__U16String *output)
{
  if (!input || !output) {
    return false;
  }
  
  // Initialize output if needed
  if (!output->data && !rosidl_runtime_c__U16String__init(output)) {
    return false;
  }
  
  // Simple mock copy - just copy the pointer for fuzzing
  // In real implementation, this would copy the actual string data
  output->size = input->size;
  output->capacity = input->capacity;
  
  return true;
}

// Fuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  // Ensure we have enough data for basic operations
  if (size < 4) {
    return 0;
  }

  // Use the first 4 bytes to determine sequence size (bounded for safety)
  size_t seq_size = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
  seq_size = seq_size % 256;  // Bound to reasonable size for fuzzing
  
  // Skip the first 4 bytes for string data
  const uint8_t *string_data = data + 4;
  size_t string_data_size = (size >= 4) ? size - 4 : 0;

  // Test 1: Create a sequence using rosidl_runtime_c__U16String__Sequence__create
  rosidl_runtime_c__U16String__Sequence *seq1 = 
    rosidl_runtime_c__U16String__Sequence__create(seq_size);
  if (!seq1) {
    // Creation failed, but that's OK for fuzzing
    return 0;
  }

  // Test 2: Initialize another sequence using rosidl_runtime_c__U16String__Sequence__init
  rosidl_runtime_c__U16String__Sequence seq2;
  bool init_success = rosidl_runtime_c__U16String__Sequence__init(&seq2, seq_size / 2);
  
  if (init_success) {
    // Test 3: Copy from seq1 to seq2 using rosidl_runtime_c__U16String__Sequence__copy
    bool copy_success = rosidl_runtime_c__U16String__Sequence__copy(seq1, &seq2);
    (void)copy_success;  // Result doesn't matter for fuzzing, just exercise the API
    
    // Test 4: Finalize seq2 using rosidl_runtime_c__U16String__Sequence__fini
    rosidl_runtime_c__U16String__Sequence__fini(&seq2);
  }

  // Test 5: Destroy seq1 using rosidl_runtime_c__U16String__Sequence__destroy
  rosidl_runtime_c__U16String__Sequence__destroy(seq1);

  // Additional test: Create, copy, and destroy with different sizes
  if (string_data_size > 0) {
    // Use another portion of data to determine size
    size_t seq3_size = string_data[0] % 128;
    
    rosidl_runtime_c__U16String__Sequence *seq3 = 
      rosidl_runtime_c__U16String__Sequence__create(seq3_size);
    
    if (seq3) {
      rosidl_runtime_c__U16String__Sequence seq4;
      if (rosidl_runtime_c__U16String__Sequence__init(&seq4, seq3_size / 2 + 1)) {
        rosidl_runtime_c__U16String__Sequence__copy(seq3, &seq4);
        rosidl_runtime_c__U16String__Sequence__fini(&seq4);
      }
      
      rosidl_runtime_c__U16String__Sequence__destroy(seq3);
    }
  }

  return 0;
}
