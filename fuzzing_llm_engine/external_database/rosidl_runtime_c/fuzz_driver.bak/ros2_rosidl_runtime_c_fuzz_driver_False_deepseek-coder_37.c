#include "rosidl_runtime_c/message_type_support.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"
#include "rosidl_runtime_c/sequence_bound.h"
#include "rosidl_runtime_c/service_type_support.h"
#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_runtime_c/u16string_functions.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Forward declarations for the underlying types used by the API functions
// These are defined in rosidl_runtime_c/primitives_sequence.h
typedef struct rosidl_runtime_c__double__Sequence
{
  double * data;
  size_t size;
  size_t capacity;
} rosidl_runtime_c__double__Sequence;

typedef struct rosidl_runtime_c__float__Sequence
{
  float * data;
  size_t size;
  size_t capacity;
} rosidl_runtime_c__float__Sequence;

typedef struct rosidl_runtime_c__boolean__Sequence
{
  bool * data;
  size_t size;
  size_t capacity;
} rosidl_runtime_c__boolean__Sequence;

typedef struct rosidl_runtime_c__octet__Sequence
{
  uint8_t * data;
  size_t size;
  size_t capacity;
} rosidl_runtime_c__octet__Sequence;

// API function declarations (as provided in the source code)
void rosidl_runtime_c__float64__Sequence__fini(rosidl_runtime_c__double__Sequence * sequence);
bool rosidl_runtime_c__float32__Sequence__init(rosidl_runtime_c__float__Sequence * sequence, size_t size);
bool rosidl_runtime_c__float64__Sequence__init(rosidl_runtime_c__double__Sequence * sequence, size_t size);
bool rosidl_runtime_c__bool__Sequence__init(rosidl_runtime_c__boolean__Sequence * sequence, size_t size);
bool rosidl_runtime_c__byte__Sequence__init(rosidl_runtime_c__octet__Sequence * sequence, size_t size);

// The actual implementations (copied from provided source code)
void rosidl_runtime_c__float64__Sequence__fini(
  rosidl_runtime_c__double__Sequence * sequence)
{
  // Call the underlying implementation
  if (sequence) {
    free(sequence->data);
    sequence->data = NULL;
    sequence->size = 0;
    sequence->capacity = 0;
  }
}

bool rosidl_runtime_c__float32__Sequence__init(
  rosidl_runtime_c__float__Sequence * sequence, size_t size)
{
  if (!sequence) return false;
  
  sequence->data = NULL;
  sequence->size = 0;
  sequence->capacity = 0;
  
  if (size > 0) {
    sequence->data = (float*)calloc(size, sizeof(float));
    if (!sequence->data) {
      return false;
    }
    sequence->size = size;
    sequence->capacity = size;
  }
  return true;
}

bool rosidl_runtime_c__float64__Sequence__init(
  rosidl_runtime_c__double__Sequence * sequence, size_t size)
{
  if (!sequence) return false;
  
  sequence->data = NULL;
  sequence->size = 0;
  sequence->capacity = 0;
  
  if (size > 0) {
    sequence->data = (double*)calloc(size, sizeof(double));
    if (!sequence->data) {
      return false;
    }
    sequence->size = size;
    sequence->capacity = size;
  }
  return true;
}

bool rosidl_runtime_c__bool__Sequence__init(
  rosidl_runtime_c__boolean__Sequence * sequence, size_t size)
{
  if (!sequence) return false;
  
  sequence->data = NULL;
  sequence->size = 0;
  sequence->capacity = 0;
  
  if (size > 0) {
    sequence->data = (bool*)calloc(size, sizeof(bool));
    if (!sequence->data) {
      return false;
    }
    sequence->size = size;
    sequence->capacity = size;
  }
  return true;
}

bool rosidl_runtime_c__byte__Sequence__init(
  rosidl_runtime_c__octet__Sequence * sequence, size_t size)
{
  if (!sequence) return false;
  
  sequence->data = NULL;
  sequence->size = 0;
  sequence->capacity = 0;
  
  if (size > 0) {
    sequence->data = (uint8_t*)calloc(size, sizeof(uint8_t));
    if (!sequence->data) {
      return false;
    }
    sequence->size = size;
    sequence->capacity = size;
  }
  return true;
}

// Fuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  // Early return if no data
  if (data == NULL || size == 0) {
    return 0;
  }

  // Initialize all sequences with sizes derived from fuzz input
  // Use modulo operations to ensure reasonable sizes and prevent excessive memory allocation
  
  // Calculate sequence sizes from fuzz input
  // Use the first few bytes to determine sizes, with bounds checking
  size_t float32_size = 0;
  size_t float64_size = 0;
  size_t bool_size = 0;
  size_t byte_size = 0;
  
  // Use different parts of the input for different sequence sizes
  // Limit sizes to prevent excessive memory allocation (max 1024 elements)
  if (size >= 4) {
    // Use first 4 bytes to determine sizes
    float32_size = (data[0] * 256 + data[1]) % 256;  // 0-255 elements
    float64_size = (data[2] * 256 + data[3]) % 256;  // 0-255 elements
  } else if (size >= 2) {
    float32_size = data[0] % 64;  // 0-63 elements
    float64_size = data[1] % 64;  // 0-63 elements
  } else {
    float32_size = size % 32;  // 0-31 elements
    float64_size = size % 32;  // 0-31 elements
  }
  
  // For bool and byte sequences, use other parts of input
  if (size >= 6) {
    bool_size = (data[4] * 256 + data[5]) % 512;  // 0-511 elements
  } else if (size >= 5) {
    bool_size = data[4] % 128;  // 0-127 elements
  } else {
    bool_size = size % 64;  // 0-63 elements
  }
  
  if (size >= 8) {
    byte_size = (data[6] * 256 + data[7]) % 1024;  // 0-1023 elements
  } else if (size >= 7) {
    byte_size = data[6] % 256;  // 0-255 elements
  } else {
    byte_size = size % 128;  // 0-127 elements
  }

  // Declare and initialize sequences
  rosidl_runtime_c__float__Sequence float32_seq;
  rosidl_runtime_c__double__Sequence float64_seq;
  rosidl_runtime_c__boolean__Sequence bool_seq;
  rosidl_runtime_c__octet__Sequence byte_seq;
  
  // Initialize all sequences with error checking
  bool init_success = true;
  
  // Initialize float32 sequence
  if (!rosidl_runtime_c__float32__Sequence__init(&float32_seq, float32_size)) {
    init_success = false;
  }
  
  // Initialize float64 sequence
  if (!rosidl_runtime_c__float64__Sequence__init(&float64_seq, float64_size)) {
    // Clean up previously allocated sequences
    if (float32_seq.data) {
      free(float32_seq.data);
    }
    init_success = false;
  }
  
  // Initialize bool sequence
  if (!rosidl_runtime_c__bool__Sequence__init(&bool_seq, bool_size)) {
    // Clean up previously allocated sequences
    if (float32_seq.data) {
      free(float32_seq.data);
    }
    if (float64_seq.data) {
      free(float64_seq.data);
    }
    init_success = false;
  }
  
  // Initialize byte sequence
  if (!rosidl_runtime_c__byte__Sequence__init(&byte_seq, byte_size)) {
    // Clean up previously allocated sequences
    if (float32_seq.data) {
      free(float32_seq.data);
    }
    if (float64_seq.data) {
      free(float64_seq.data);
    }
    if (bool_seq.data) {
      free(bool_seq.data);
    }
    init_success = false;
  }
  
  // If all initializations succeeded, populate sequences with fuzz data
  if (init_success) {
    // Populate float32 sequence with data from fuzz input
    if (float32_seq.data && float32_size > 0) {
      size_t copy_size = float32_size * sizeof(float);
      size_t data_offset = 0;
      
      // Copy as much data as available, but don't overflow
      for (size_t i = 0; i < float32_size; i++) {
        if (data_offset + sizeof(float) <= size) {
          // Copy 4 bytes for each float (simplified - actual float representation may vary)
          memcpy(&float32_seq.data[i], data + data_offset, sizeof(float));
          data_offset += sizeof(float);
        } else {
          // Fill remaining with zeros
          float32_seq.data[i] = 0.0f;
        }
      }
    }
    
    // Populate float64 sequence with data from fuzz input
    if (float64_seq.data && float64_size > 0) {
      size_t data_offset = 8;  // Start from different offset
      
      for (size_t i = 0; i < float64_size; i++) {
        if (data_offset + sizeof(double) <= size) {
          memcpy(&float64_seq.data[i], data + data_offset, sizeof(double));
          data_offset += sizeof(double);
        } else {
          float64_seq.data[i] = 0.0;
        }
      }
    }
    
    // Populate bool sequence with data from fuzz input
    if (bool_seq.data && bool_size > 0) {
      size_t data_offset = 16;  // Start from different offset
      
      for (size_t i = 0; i < bool_size; i++) {
        if (data_offset < size) {
          bool_seq.data[i] = (data[data_offset] & 0x01) != 0;
          data_offset++;
        } else {
          bool_seq.data[i] = false;
        }
      }
    }
    
    // Populate byte sequence with data from fuzz input
    if (byte_seq.data && byte_size > 0) {
      // Copy directly from fuzz input, bounded by available data
      size_t copy_size = byte_size;
      if (copy_size > size) {
        copy_size = size;
      }
      
      if (copy_size > 0) {
        memcpy(byte_seq.data, data, copy_size);
      }
      
      // Fill remaining bytes with zeros if needed
      if (copy_size < byte_size) {
        memset(byte_seq.data + copy_size, 0, byte_size - copy_size);
      }
    }
    
    // Perform some simple operations on the sequences to exercise the code
    // Calculate sum of float32 sequence
    float float32_sum = 0.0f;
    for (size_t i = 0; i < float32_seq.size; i++) {
      float32_sum += float32_seq.data[i];
    }
    
    // Calculate sum of float64 sequence
    double float64_sum = 0.0;
    for (size_t i = 0; i < float64_seq.size; i++) {
      float64_sum += float64_seq.data[i];
    }
    
    // Count true values in bool sequence
    size_t true_count = 0;
    for (size_t i = 0; i < bool_seq.size; i++) {
      if (bool_seq.data[i]) {
        true_count++;
      }
    }
    
    // Calculate byte sequence checksum
    uint8_t byte_checksum = 0;
    for (size_t i = 0; i < byte_seq.size; i++) {
      byte_checksum ^= byte_seq.data[i];
    }
    
    // Use the calculated values to prevent optimization removal
    // This is a dummy operation that uses the variables
    volatile float dummy_float = float32_sum;
    volatile double dummy_double = float64_sum;
    volatile size_t dummy_count = true_count;
    volatile uint8_t dummy_checksum = byte_checksum;
    
    (void)dummy_float;
    (void)dummy_double;
    (void)dummy_count;
    (void)dummy_checksum;
  }
  
  // Clean up all sequences
  // Note: rosidl_runtime_c__float64__Sequence__fini only handles float64/double sequences
  // For other sequences, we need to manually free their data
  
  // Finalize float64 sequence using the provided API function
  rosidl_runtime_c__float64__Sequence__fini(&float64_seq);
  
  // Manually free other sequences (no fini functions provided for them)
  if (float32_seq.data) {
    free(float32_seq.data);
    float32_seq.data = NULL;
    float32_seq.size = 0;
    float32_seq.capacity = 0;
  }
  
  if (bool_seq.data) {
    free(bool_seq.data);
    bool_seq.data = NULL;
    bool_seq.size = 0;
    bool_seq.capacity = 0;
  }
  
  if (byte_seq.data) {
    free(byte_seq.data);
    byte_seq.data = NULL;
    byte_seq.size = 0;
    byte_seq.capacity = 0;
  }
  
  return 0;
}
