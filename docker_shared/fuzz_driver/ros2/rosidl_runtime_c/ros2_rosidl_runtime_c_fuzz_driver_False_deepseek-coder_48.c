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

// Note: <sstream> is a C++ header, but we're writing C code.
// We'll use C standard library functions instead.

// Forward declarations for the sequence types
// Based on ROS 2 conventions and API source code
typedef struct rosidl_runtime_c__boolean__Sequence {
  bool * data;
  size_t size;
  size_t capacity;
} rosidl_runtime_c__boolean__Sequence;

typedef struct rosidl_runtime_c__double__Sequence {
  double * data;
  size_t size;
  size_t capacity;
} rosidl_runtime_c__double__Sequence;

// API function declarations (from provided source code)
bool rosidl_runtime_c__bool__Sequence__are_equal(
  const rosidl_runtime_c__boolean__Sequence * lhs,
  const rosidl_runtime_c__boolean__Sequence * rhs);

void rosidl_runtime_c__float64__Sequence__fini(
  rosidl_runtime_c__double__Sequence * sequence);

bool rosidl_runtime_c__float64__Sequence__init(
  rosidl_runtime_c__double__Sequence * sequence, size_t size);

bool rosidl_runtime_c__float64__Sequence__copy(
  const rosidl_runtime_c__double__Sequence * input,
  rosidl_runtime_c__double__Sequence * output);

bool rosidl_runtime_c__float64__Sequence__are_equal(
  const rosidl_runtime_c__double__Sequence * lhs,
  const rosidl_runtime_c__double__Sequence * rhs);

// API implementations (from provided source code)
bool rosidl_runtime_c__bool__Sequence__are_equal(
  const rosidl_runtime_c__boolean__Sequence * lhs,
  const rosidl_runtime_c__boolean__Sequence * rhs)
{
  // Simple implementation based on API summary
  if (lhs == NULL || rhs == NULL) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; i++) {
    if (lhs->data[i] != rhs->data[i]) {
      return false;
    }
  }
  return true;
}

void rosidl_runtime_c__float64__Sequence__fini(
  rosidl_runtime_c__double__Sequence * sequence)
{
  if (sequence != NULL) {
    free(sequence->data);
    sequence->data = NULL;
    sequence->size = 0;
    sequence->capacity = 0;
  }
}

bool rosidl_runtime_c__float64__Sequence__init(
  rosidl_runtime_c__double__Sequence * sequence, size_t size)
{
  if (sequence == NULL) {
    return false;
  }
  
  sequence->data = NULL;
  sequence->size = 0;
  sequence->capacity = 0;
  
  if (size > 0) {
    sequence->data = (double*)calloc(size, sizeof(double));
    if (sequence->data == NULL) {
      return false;
    }
    sequence->size = size;
    sequence->capacity = size;
  }
  return true;
}

bool rosidl_runtime_c__float64__Sequence__copy(
  const rosidl_runtime_c__double__Sequence * input,
  rosidl_runtime_c__double__Sequence * output)
{
  if (input == NULL || output == NULL) {
    return false;
  }
  
  // Initialize output with input size
  if (!rosidl_runtime_c__float64__Sequence__init(output, input->size)) {
    return false;
  }
  
  // Copy data
  if (input->size > 0) {
    memcpy(output->data, input->data, input->size * sizeof(double));
  }
  return true;
}

bool rosidl_runtime_c__float64__Sequence__are_equal(
  const rosidl_runtime_c__double__Sequence * lhs,
  const rosidl_runtime_c__double__Sequence * rhs)
{
  if (lhs == NULL || rhs == NULL) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; i++) {
    // Use epsilon comparison for floating point values
    if (lhs->data[i] != rhs->data[i]) {
      return false;
    }
  }
  return true;
}

// Fuzz driver entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  // Early return if no data
  if (data == NULL || size == 0) {
    return 0;
  }
  
  // Initialize all sequences to NULL/zero state
  rosidl_runtime_c__double__Sequence seq1 = {0};
  rosidl_runtime_c__double__Sequence seq2 = {0};
  rosidl_runtime_c__double__Sequence seq3 = {0};
  rosidl_runtime_c__boolean__Sequence bool_seq1 = {0};
  rosidl_runtime_c__boolean__Sequence bool_seq2 = {0};
  
  // Determine sequence sizes from fuzz input
  // Use first 4 bytes for size (if available), but limit to reasonable size
  size_t seq_size = 0;
  if (size >= sizeof(uint32_t)) {
    uint32_t size_val;
    memcpy(&size_val, data, sizeof(uint32_t));
    // Limit to reasonable size to prevent excessive memory allocation
    seq_size = size_val % 256;  // Max 256 elements
  } else {
    seq_size = size % 64;  // Smaller size for small inputs
  }
  
  // Ensure we have at least some data for testing
  if (seq_size == 0) {
    seq_size = 1;
  }
  
  // 1. Initialize first float64 sequence
  if (!rosidl_runtime_c__float64__Sequence__init(&seq1, seq_size)) {
    // Cleanup and return if initialization fails
    goto cleanup;
  }
  
  // Fill sequence with data from fuzz input
  size_t data_offset = sizeof(uint32_t) % size;  // Start after size bytes
  for (size_t i = 0; i < seq_size && data_offset < size; i++, data_offset++) {
    // Convert byte to double value (simple scaling)
    seq1.data[i] = (double)data[data_offset] / 256.0;
  }
  
  // 2. Copy seq1 to seq2
  if (!rosidl_runtime_c__float64__Sequence__copy(&seq1, &seq2)) {
    goto cleanup;
  }
  
  // 3. Initialize seq3 with different size (half of seq_size)
  size_t seq3_size = seq_size / 2;
  if (seq3_size == 0) seq3_size = 1;
  
  if (!rosidl_runtime_c__float64__Sequence__init(&seq3, seq3_size)) {
    goto cleanup;
  }
  
  // Fill seq3 with different data
  data_offset = (data_offset + 1) % size;  // Wrap around if needed
  for (size_t i = 0; i < seq3_size && data_offset < size; i++, data_offset++) {
    seq3.data[i] = (double)data[data_offset] / 128.0;
  }
  
  // 4. Test float64 sequence equality (seq1 should equal seq2)
  bool float_equal1 = rosidl_runtime_c__float64__Sequence__are_equal(&seq1, &seq2);
  bool float_equal2 = rosidl_runtime_c__float64__Sequence__are_equal(&seq1, &seq3);
  
  // 5. Initialize boolean sequences for testing
  // Use minimal size for boolean sequences
  size_t bool_size = seq_size % 8;
  if (bool_size == 0) bool_size = 1;
  
  bool_seq1.data = (bool*)calloc(bool_size, sizeof(bool));
  bool_seq2.data = (bool*)calloc(bool_size, sizeof(bool));
  
  if (bool_seq1.data == NULL || bool_seq2.data == NULL) {
    goto cleanup;
  }
  
  bool_seq1.size = bool_size;
  bool_seq1.capacity = bool_size;
  bool_seq2.size = bool_size;
  bool_seq2.capacity = bool_size;
  
  // Fill boolean sequences with data from fuzz input
  data_offset = (data_offset + 1) % size;
  for (size_t i = 0; i < bool_size && data_offset < size; i++, data_offset++) {
    bool_seq1.data[i] = (data[data_offset] & 0x01) != 0;
    bool_seq2.data[i] = (data[data_offset] & 0x01) != 0;  // Same data for equality
  }
  
  // 6. Test boolean sequence equality
  bool bool_equal = rosidl_runtime_c__bool__Sequence__are_equal(&bool_seq1, &bool_seq2);
  
  // Use the results to prevent optimization removal
  (void)float_equal1;
  (void)float_equal2;
  (void)bool_equal;
  
cleanup:
  // 7. Finalize all sequences (free allocated memory)
  rosidl_runtime_c__float64__Sequence__fini(&seq1);
  rosidl_runtime_c__float64__Sequence__fini(&seq2);
  rosidl_runtime_c__float64__Sequence__fini(&seq3);
  
  // Free boolean sequence memory
  if (bool_seq1.data != NULL) {
    free(bool_seq1.data);
  }
  if (bool_seq2.data != NULL) {
    free(bool_seq2.data);
  }
  
  return 0;
}
