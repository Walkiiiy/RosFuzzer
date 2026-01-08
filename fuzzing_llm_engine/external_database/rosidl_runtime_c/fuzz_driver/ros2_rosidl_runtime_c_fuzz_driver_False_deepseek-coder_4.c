#include <rosidl_runtime_c/action_type_support_struct.h>
#include <rosidl_runtime_c/message_initialization.h>
#include <rosidl_runtime_c/message_type_support_struct.h>
#include <rosidl_runtime_c/primitives_sequence_functions.h>
#include <rosidl_runtime_c/primitives_sequence.h>
#include <rosidl_runtime_c/sequence_bound.h>
#include <rosidl_runtime_c/service_type_support_struct.h>
#include <rosidl_runtime_c/string_functions.h>
#include <rosidl_runtime_c/u16string_functions.h>
#include <rosidl_runtime_c/visibility_control.h>
#include <rosidl_runtime_c/string.h>
#include <rosidl_runtime_c/string_bound.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sstream>
#include <rcutils/allocator.h>

// Function prototypes for APIs that might not be directly declared in headers
// Based on the provided source code
bool rosidl_runtime_c__bool__Sequence__are_equal(
  const rosidl_runtime_c__boolean__Sequence * lhs,
  const rosidl_runtime_c__boolean__Sequence * rhs);

const rosidl_runtime_c__Sequence__bound * get_sequence_bound_handle_function(
  const rosidl_runtime_c__Sequence__bound * handle, const char * identifier);

bool rosidl_runtime_c__String__Sequence__init(
  rosidl_runtime_c__String__Sequence * sequence, size_t size);

void rosidl_runtime_c__String__fini(rosidl_runtime_c__String * str);

const rosidl_service_type_support_t * get_service_typesupport_handle(
  const rosidl_service_type_support_t * handle, const char * identifier);

// Main fuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  // Early exit for insufficient input
  if (size < 4) {
    return 0;
  }

  // Initialize default allocator
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  
  // Use first byte to determine test scenario
  uint8_t scenario = data[0];
  size_t offset = 1;
  
  // Test 1: rosidl_runtime_c__String__Sequence__init and rosidl_runtime_c__String__fini
  if (scenario & 0x01) {
    // Determine sequence size from input (limit to reasonable size)
    size_t seq_size = 0;
    if (size - offset >= sizeof(size_t)) {
      // Use next 4 bytes for size, but limit to prevent excessive allocation
      memcpy(&seq_size, data + offset, sizeof(size_t));
      offset += sizeof(size_t);
      seq_size = seq_size % 256;  // Limit to prevent OOM
    } else {
      // Use remaining bytes as size
      seq_size = (size - offset) % 256;
    }
    
    rosidl_runtime_c__String__Sequence string_seq;
    
    // Initialize string sequence
    bool init_success = rosidl_runtime_c__String__Sequence__init(&string_seq, seq_size);
    
    if (init_success) {
      // If initialization succeeded, populate strings with fuzz data
      size_t data_idx = offset;
      for (size_t i = 0; i < string_seq.size && data_idx < size; i++) {
        // Determine string length from fuzz data (limit to prevent overflow)
        size_t str_len = data[data_idx] % 64 + 1;  // 1-64 bytes
        data_idx = (data_idx + 1) % size;
        
        // Allocate memory for string
        string_seq.data[i].data = (char*)allocator.allocate(str_len + 1, allocator.state);
        if (string_seq.data[i].data) {
          string_seq.data[i].size = 0;
          string_seq.data[i].capacity = str_len + 1;
          
          // Copy fuzz data to string (safe copy)
          size_t copy_len = str_len;
          if (data_idx + copy_len > size) {
            copy_len = size - data_idx;
          }
          if (copy_len > 0) {
            memcpy(string_seq.data[i].data, data + data_idx, copy_len);
            string_seq.data[i].data[copy_len] = '\0';
            string_seq.data[i].size = copy_len;
            data_idx += copy_len;
          } else {
            string_seq.data[i].data[0] = '\0';
          }
        }
      }
      
      // Clean up: finalize each string and free sequence
      for (size_t i = 0; i < string_seq.size; i++) {
        rosidl_runtime_c__String__fini(&string_seq.data[i]);
      }
      allocator.deallocate(string_seq.data, allocator.state);
    }
  }
  
  // Test 2: rosidl_runtime_c__bool__Sequence__are_equal
  if (scenario & 0x02 && size - offset >= 2) {
    // Create two boolean sequences for comparison
    rosidl_runtime_c__boolean__Sequence bool_seq1;
    rosidl_runtime_c__boolean__Sequence bool_seq2;
    
    // Initialize sequences with reasonable sizes
    size_t bool_size = data[offset] % 16 + 1;  // 1-16 elements
    offset = (offset + 1) % size;
    
    bool_seq1.size = bool_size;
    bool_seq2.size = bool_size;
    
    // Allocate memory for boolean sequences
    bool_seq1.data = (bool*)allocator.allocate(bool_size * sizeof(bool), allocator.state);
    bool_seq2.data = (bool*)allocator.allocate(bool_size * sizeof(bool), allocator.state);
    
    if (bool_seq1.data && bool_seq2.data) {
      // Populate sequences with fuzz data
      for (size_t i = 0; i < bool_size && offset < size; i++) {
        bool_seq1.data[i] = (data[offset] & 0x01) != 0;
        bool_seq2.data[i] = (data[offset] & 0x02) != 0;
        offset = (offset + 1) % size;
      }
      
      // Compare the sequences
      bool are_equal = rosidl_runtime_c__bool__Sequence__are_equal(&bool_seq1, &bool_seq2);
      (void)are_equal;  // Use result to avoid unused variable warning
      
      // Clean up
      allocator.deallocate(bool_seq1.data, allocator.state);
      allocator.deallocate(bool_seq2.data, allocator.state);
    } else {
      // Clean up partial allocations
      if (bool_seq1.data) allocator.deallocate(bool_seq1.data, allocator.state);
      if (bool_seq2.data) allocator.deallocate(bool_seq2.data, allocator.state);
    }
  }
  
  // Test 3: get_sequence_bound_handle_function
  if (scenario & 0x04 && size - offset >= 1) {
    rosidl_runtime_c__Sequence__bound bound_handle;
    
    // Create identifier from fuzz data
    char identifier[64];
    size_t id_len = data[offset] % 63 + 1;  // 1-63 chars + null terminator
    offset = (offset + 1) % size;
    
    // Copy identifier safely
    size_t copy_len = id_len;
    if (offset + copy_len > size) {
      copy_len = size - offset;
    }
    if (copy_len > 0) {
      memcpy(identifier, data + offset, copy_len);
      identifier[copy_len] = '\0';
      offset += copy_len;
    } else {
      identifier[0] = '\0';
    }
    
    // Set up the handle
    bound_handle.typesupport_identifier = identifier;
    
    // Call the function
    const rosidl_runtime_c__Sequence__bound * result = 
      get_sequence_bound_handle_function(&bound_handle, identifier);
    (void)result;  // Use result to avoid unused variable warning
  }
  
  // Test 4: get_service_typesupport_handle
  if (scenario & 0x08 && size - offset >= 1) {
    // Create a mock service type support structure
    rosidl_service_type_support_t service_handle;
    
    // Create identifier from fuzz data
    char service_identifier[64];
    size_t service_id_len = data[offset] % 63 + 1;
    offset = (offset + 1) % size;
    
    // Copy identifier safely
    size_t copy_len = service_id_len;
    if (offset + copy_len > size) {
      copy_len = size - offset;
    }
    if (copy_len > 0) {
      memcpy(service_identifier, data + offset, copy_len);
      service_identifier[copy_len] = '\0';
      offset += copy_len;
    } else {
      service_identifier[0] = '\0';
    }
    
    // We need to provide a valid function pointer. Since we can't mock,
    // we'll create a simple function that returns NULL.
    // Note: In a real fuzzer, we would link against the actual library.
    service_handle.func = NULL;
    
    // Call the function (will likely return NULL due to NULL func)
    const rosidl_service_type_support_t * service_result = 
      get_service_typesupport_handle(&service_handle, service_identifier);
    (void)service_result;  // Use result to avoid unused variable warning
  }
  
  return 0;
}
