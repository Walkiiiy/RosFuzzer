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
#include <rcutils/allocator.h>

// LLVMFuzzerTestOneInput function signature
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

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
  
  // Test 1: rosidl_runtime_c__String__Sequence APIs
  if (scenario & 0x01 && size - offset >= 1) {
    // Determine sequence size from input (limit to reasonable size)
    size_t seq_size = data[offset] % 16 + 1;  // 1-16 elements
    offset = (offset + 1) % size;
    
    rosidl_runtime_c__String__Sequence string_seq;
    
    // Initialize string sequence
    bool init_success = rosidl_runtime_c__String__Sequence__init(&string_seq, seq_size);
    
    if (init_success) {
      // If initialization succeeded, populate strings with fuzz data
      for (size_t i = 0; i < string_seq.size && offset < size; i++) {
        // Determine string length from fuzz data
        size_t str_len = data[offset] % 32 + 1;  // 1-32 bytes
        offset = (offset + 1) % size;
        
        // IMPORTANT: Save the original allocated data pointer
        char *original_data = string_seq.data[i].data;
        
        // Resize the string to hold our data
        if (str_len > 0) {
          // We need to allocate new memory for our string data
          char *new_data = (char*)allocator.reallocate(
            string_seq.data[i].data, 
            str_len + 1, 
            allocator.state
          );
          
          if (new_data) {
            string_seq.data[i].data = new_data;
            string_seq.data[i].capacity = str_len + 1;
            
            // Copy fuzz data to string
            size_t copy_len = str_len;
            if (offset + copy_len > size) {
              copy_len = size - offset;
            }
            if (copy_len > 0) {
              memcpy(string_seq.data[i].data, data + offset, copy_len);
              string_seq.data[i].data[copy_len] = '\0';
              string_seq.data[i].size = copy_len;
              offset += copy_len;
            } else {
              string_seq.data[i].data[0] = '\0';
              string_seq.data[i].size = 0;
            }
          }
        }
      }
      
      // Test copy function
      rosidl_runtime_c__String__Sequence string_seq_copy;
      if (rosidl_runtime_c__String__Sequence__init(&string_seq_copy, seq_size)) {
        bool copy_success = rosidl_runtime_c__String__Sequence__copy(&string_seq, &string_seq_copy);
        (void)copy_success;  // Use to avoid warning
        
        // Test are_equal function
        bool are_equal = rosidl_runtime_c__String__Sequence__are_equal(&string_seq, &string_seq_copy);
        (void)are_equal;
        
        // Clean up copy
        rosidl_runtime_c__String__Sequence__fini(&string_seq_copy);
      }
      
      // Test create/destroy APIs
      rosidl_runtime_c__String__Sequence *dynamic_seq = 
        rosidl_runtime_c__String__Sequence__create(seq_size);
      if (dynamic_seq) {
        // Test with NULL parameters (should be safe)
        rosidl_runtime_c__String__Sequence__are_equal(NULL, dynamic_seq);
        rosidl_runtime_c__String__Sequence__are_equal(dynamic_seq, NULL);
        rosidl_runtime_c__String__Sequence__are_equal(NULL, NULL);
        
        rosidl_runtime_c__String__Sequence__copy(NULL, dynamic_seq);
        rosidl_runtime_c__String__Sequence__copy(dynamic_seq, NULL);
        
        rosidl_runtime_c__String__Sequence__destroy(dynamic_seq);
      }
      
      // CORRECT CLEANUP: Use fini, not manual deallocation
      rosidl_runtime_c__String__Sequence__fini(&string_seq);
    }
  }
  
  // Test 2: Primitives sequence functions
  if (scenario & 0x02 && size - offset >= 1) {
    // Test different primitive types
    uint8_t seq_type = data[offset] % 4;
    offset = (offset + 1) % size;
    
    size_t seq_size = data[offset] % 8 + 1;
    offset = (offset + 1) % size;
    
    switch (seq_type) {
      case 0: { // bool sequence
        rosidl_runtime_c__boolean__Sequence bool_seq;
        if (rosidl_runtime_c__boolean__Sequence__init(&bool_seq, seq_size)) {
          // Fill with fuzz data
          for (size_t i = 0; i < bool_seq.size && offset < size; i++) {
            bool_seq.data[i] = (data[offset] & 0x01) != 0;
            offset = (offset + 1) % size;
          }
          rosidl_runtime_c__boolean__Sequence__fini(&bool_seq);
        }
        break;
      }
      case 1: { // int32 sequence
        rosidl_runtime_c__int32__Sequence int_seq;
        if (rosidl_runtime_c__int32__Sequence__init(&int_seq, seq_size)) {
          // Fill with fuzz data
          for (size_t i = 0; i < int_seq.size && offset + 3 < size; i++) {
            int32_t value;
            memcpy(&value, data + offset, sizeof(int32_t));
            int_seq.data[i] = value;
            offset = (offset + 4) % size;
          }
          rosidl_runtime_c__int32__Sequence__fini(&int_seq);
        }
        break;
      }
      case 2: { // double sequence
        rosidl_runtime_c__double__Sequence double_seq;
        if (rosidl_runtime_c__double__Sequence__init(&double_seq, seq_size)) {
          // Fill with fuzz data
          for (size_t i = 0; i < double_seq.size && offset + 7 < size; i++) {
            double value;
            memcpy(&value, data + offset, sizeof(double));
            double_seq.data[i] = value;
            offset = (offset + 8) % size;
          }
          rosidl_runtime_c__double__Sequence__fini(&double_seq);
        }
        break;
      }
      case 3: { // string functions (single string)
        rosidl_runtime_c__String single_string;
        rosidl_runtime_c__String__init(&single_string);
        
        // Set string data
        size_t str_len = data[offset] % 64 + 1;
        offset = (offset + 1) % size;
        
        if (str_len > 0) {
          char *new_data = (char*)allocator.reallocate(
            single_string.data,
            str_len + 1,
            allocator.state
          );
          if (new_data) {
            single_string.data = new_data;
            single_string.capacity = str_len + 1;
            
            size_t copy_len = str_len;
            if (offset + copy_len > size) {
              copy_len = size - offset;
            }
            if (copy_len > 0) {
              memcpy(single_string.data, data + offset, copy_len);
              single_string.data[copy_len] = '\0';
              single_string.size = copy_len;
              offset += copy_len;
            } else {
              single_string.data[0] = '\0';
              single_string.size = 0;
            }
          }
        }
        
        // Test copy
        rosidl_runtime_c__String string_copy;
        rosidl_runtime_c__String__init(&string_copy);
        rosidl_runtime_c__String__copy(&single_string, &string_copy);
        
        // Test assign
        rosidl_runtime_c__String__assign(&string_copy, "test");
        
        // Cleanup
        rosidl_runtime_c__String__fini(&single_string);
        rosidl_runtime_c__String__fini(&string_copy);
        break;
      }
    }
  }
  
  // Test 3: Sequence boundary functions
  if (scenario & 0x04 && size - offset >= 1) {
    // Test with reasonable values
    size_t bound_size = data[offset] % 1000;  // 0-999
    offset = (offset + 1) % size;
    
    // Create sequence with bound
    rosidl_runtime_c__String__Sequence bounded_seq;
    if (rosidl_runtime_c__String__Sequence__init(&bounded_seq, bound_size)) {
      // Test various operations within bounds
      for (size_t i = 0; i < bounded_seq.size && i < 10 && offset < size; i++) {
        size_t str_len = data[offset] % 16 + 1;
        offset = (offset + 1) % size;
        
        if (str_len > 0) {
          char *new_data = (char*)allocator.reallocate(
            bounded_seq.data[i].data,
            str_len + 1,
            allocator.state
          );
          if (new_data) {
            bounded_seq.data[i].data = new_data;
            bounded_seq.data[i].capacity = str_len + 1;
            
            size_t copy_len = str_len;
            if (offset + copy_len > size) {
              copy_len = size - offset;
            }
            if (copy_len > 0) {
              memcpy(bounded_seq.data[i].data, data + offset, copy_len);
              bounded_seq.data[i].data[copy_len] = '\0';
              bounded_seq.data[i].size = copy_len;
              offset += copy_len;
            }
          }
        }
      }
      
      rosidl_runtime_c__String__Sequence__fini(&bounded_seq);
    }
  }
  
  // Test 4: Message initialization
  if (scenario & 0x08 && size - offset >= 1) {
    // Create a simple message-like structure
    typedef struct {
      int32_t id;
      rosidl_runtime_c__String name;
      double value;
    } TestMessage;
    
    TestMessage msg;
    
    // Initialize string field
    rosidl_runtime_c__String__init(&msg.name);
    
    // Set values from fuzz data
    if (offset + sizeof(int32_t) <= size) {
      memcpy(&msg.id, data + offset, sizeof(int32_t));
      offset += sizeof(int32_t);
    }
    
    if (offset + sizeof(double) <= size) {
      memcpy(&msg.value, data + offset, sizeof(double));
      offset += sizeof(double);
    }
    
    // Set string data
    if (offset < size) {
      size_t str_len = data[offset] % 32 + 1;
      offset = (offset + 1) % size;
      
      if (str_len > 0) {
        char *new_data = (char*)allocator.reallocate(
          msg.name.data,
          str_len + 1,
          allocator.state
        );
        if (new_data) {
          msg.name.data = new_data;
          msg.name.capacity = str_len + 1;
          
          size_t copy_len = str_len;
          if (offset + copy_len > size) {
            copy_len = size - offset;
          }
          if (copy_len > 0) {
            memcpy(msg.name.data, data + offset, copy_len);
            msg.name.data[copy_len] = '\0';
            msg.name.size = copy_len;
            offset += copy_len;
          }
        }
      }
    }
    
    // Cleanup
    rosidl_runtime_c__String__fini(&msg.name);
  }
  
  return 0;
}