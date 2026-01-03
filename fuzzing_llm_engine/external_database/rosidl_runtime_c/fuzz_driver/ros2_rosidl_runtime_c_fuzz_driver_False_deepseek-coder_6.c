#include <rosidl_runtime_c/message_type_support.h>
#include <rosidl_runtime_c/primitives_sequence_functions.h>
#include <rosidl_runtime_c/sequence_bound.h>
#include <rosidl_runtime_c/service_type_support.h>
#include <rosidl_runtime_c/string_functions.h>
#include <rosidl_runtime_c/u16string_functions.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <rcutils/allocator.h>

// Function prototypes for APIs that might not be directly declared in headers
// Based on the provided source code
void rosidl_runtime_c__String__fini(rosidl_runtime_c__String * str);
void rosidl_runtime_c__float64__Sequence__fini(rosidl_runtime_c__double__Sequence * sequence);
bool rosidl_runtime_c__String__Sequence__copy(
  const rosidl_runtime_c__String__Sequence * input,
  rosidl_runtime_c__String__Sequence * output);
const rosidl_service_type_support_t * get_service_typesupport_handle_function(
  const rosidl_service_type_support_t * handle, const char * identifier);
bool rosidl_runtime_c__bool__Sequence__are_equal(
  const rosidl_runtime_c__boolean__Sequence * lhs,
  const rosidl_runtime_c__boolean__Sequence * rhs);

// Helper function to initialize a rosidl_runtime_c__String from fuzz data
static bool init_string_from_fuzz(rosidl_runtime_c__String * str, const uint8_t *data, size_t size, size_t offset) {
    if (!str) return false;
    
    // Initialize the string structure
    if (!rosidl_runtime_c__String__init(str)) {
        return false;
    }
    
    // Calculate how much data we can use for this string
    // Use at most 256 bytes per string to avoid excessive allocations
    size_t str_size = (size - offset) % 256;
    if (str_size == 0 && (size - offset) > 0) {
        str_size = 1;  // At least 1 byte if we have data
    }
    
    // Ensure we don't go beyond available data
    if (offset + str_size > size) {
        str_size = size - offset;
    }
    
    if (str_size > 0) {
        // Resize the string to hold our data
        if (!rosidl_runtime_c__String__resize(str, str_size)) {
            rosidl_runtime_c__String__fini(str);
            return false;
        }
        
        // Copy the fuzz data into the string
        memcpy(str->data, data + offset, str_size);
        str->data[str_size] = '\0';  // Ensure null termination
    }
    
    return true;
}

// Helper function to initialize a float64 sequence from fuzz data
static bool init_float64_sequence_from_fuzz(rosidl_runtime_c__double__Sequence * seq, 
                                           const uint8_t *data, size_t size, size_t offset) {
    if (!seq) return false;
    
    // Initialize the sequence
    if (!rosidl_runtime_c__float64__Sequence__init(seq, 0)) {
        return false;
    }
    
    // Calculate sequence size from fuzz data (limit to reasonable size)
    size_t seq_size = (size - offset) % 64;
    
    // Ensure we have enough data for the sequence
    if (seq_size > 0 && offset + seq_size * sizeof(double) <= size) {
        // Resize the sequence
        if (!rosidl_runtime_c__float64__Sequence__resize(seq, seq_size)) {
            rosidl_runtime_c__float64__Sequence__fini(seq);
            return false;
        }
        
        // Fill with values derived from fuzz data
        for (size_t i = 0; i < seq_size; i++) {
            double value = 0.0;
            size_t data_offset = offset + (i * sizeof(double)) % (size - offset);
            memcpy(&value, data + data_offset, sizeof(double) < (size - data_offset) ? 
                   sizeof(double) : (size - data_offset));
            seq->data[i] = value;
        }
    }
    
    return true;
}

// Helper function to initialize a bool sequence from fuzz data
static bool init_bool_sequence_from_fuzz(rosidl_runtime_c__boolean__Sequence * seq,
                                        const uint8_t *data, size_t size, size_t offset) {
    if (!seq) return false;
    
    // Initialize the sequence
    if (!rosidl_runtime_c__bool__Sequence__init(seq, 0)) {
        return false;
    }
    
    // Calculate sequence size from fuzz data (limit to reasonable size)
    size_t seq_size = (size - offset) % 64;
    
    if (seq_size > 0 && offset + seq_size <= size) {
        // Resize the sequence
        if (!rosidl_runtime_c__bool__Sequence__resize(seq, seq_size)) {
            rosidl_runtime_c__bool__Sequence__fini(seq);
            return false;
        }
        
        // Fill with boolean values derived from fuzz data
        for (size_t i = 0; i < seq_size; i++) {
            size_t data_offset = (offset + i) % size;
            seq->data[i] = (data[data_offset] & 0x01) != 0;
        }
    }
    
    return true;
}

// Helper function to initialize a string sequence from fuzz data
static bool init_string_sequence_from_fuzz(rosidl_runtime_c__String__Sequence * seq,
                                          const uint8_t *data, size_t size, size_t offset) {
    if (!seq) return false;
    
    // Initialize the sequence
    if (!rosidl_runtime_c__String__Sequence__init(seq, 0)) {
        return false;
    }
    
    // Calculate sequence size from fuzz data (limit to reasonable size)
    size_t seq_size = (size - offset) % 16;
    
    if (seq_size > 0) {
        // Resize the sequence
        if (!rosidl_runtime_c__String__Sequence__resize(seq, seq_size)) {
            rosidl_runtime_c__String__Sequence__fini(seq);
            return false;
        }
        
        // Initialize each string in the sequence
        for (size_t i = 0; i < seq_size; i++) {
            // Use different offsets for each string to maximize coverage
            size_t str_offset = (offset + i * 16) % size;
            if (!init_string_from_fuzz(&seq->data[i], data, size, str_offset)) {
                // Clean up already initialized strings
                for (size_t j = 0; j < i; j++) {
                    rosidl_runtime_c__String__fini(&seq->data[j]);
                }
                rosidl_runtime_c__String__Sequence__fini(seq);
                return false;
            }
        }
    }
    
    return true;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early return if no data
    if (size == 0 || data == NULL) {
        return 0;
    }
    
    // 1. Test rosidl_runtime_c__String__fini
    rosidl_runtime_c__String test_string;
    if (init_string_from_fuzz(&test_string, data, size, 0)) {
        // Call the API
        rosidl_runtime_c__String__fini(&test_string);
        
        // Test with NULL pointer (should be safe)
        rosidl_runtime_c__String__fini(NULL);
    }
    
    // 2. Test rosidl_runtime_c__float64__Sequence__fini
    rosidl_runtime_c__double__Sequence float_seq;
    if (init_float64_sequence_from_fuzz(&float_seq, data, size, size/4)) {
        // Call the API
        rosidl_runtime_c__float64__Sequence__fini(&float_seq);
        
        // Test with uninitialized/zero sequence
        rosidl_runtime_c__double__Sequence zero_seq = {0};
        rosidl_runtime_c__float64__Sequence__fini(&zero_seq);
    }
    
    // 3. Test rosidl_runtime_c__String__Sequence__copy
    rosidl_runtime_c__String__Sequence src_seq, dst_seq;
    
    // Initialize source sequence
    if (init_string_sequence_from_fuzz(&src_seq, data, size, size/3)) {
        // Initialize destination sequence
        if (rosidl_runtime_c__String__Sequence__init(&dst_seq, 0)) {
            // Call the API
            bool copy_result = rosidl_runtime_c__String__Sequence__copy(&src_seq, &dst_seq);
            
            // Clean up destination if copy succeeded
            if (copy_result) {
                rosidl_runtime_c__String__Sequence__fini(&dst_seq);
            } else {
                // If copy failed, dst_seq might be partially initialized
                // Clean up what was initialized
                rosidl_runtime_c__String__Sequence__fini(&dst_seq);
            }
            
            // Test with NULL pointers
            bool null_test1 = rosidl_runtime_c__String__Sequence__copy(NULL, &dst_seq);
            bool null_test2 = rosidl_runtime_c__String__Sequence__copy(&src_seq, NULL);
            bool null_test3 = rosidl_runtime_c__String__Sequence__copy(NULL, NULL);
            (void)null_test1; (void)null_test2; (void)null_test3; // Suppress unused warnings
        }
        
        // Clean up source sequence
        rosidl_runtime_c__String__Sequence__fini(&src_seq);
    }
    
    // 4. Test get_service_typesupport_handle_function
    // Create a mock service type support structure
    static const char test_identifier[] = "rosidl_typesupport_c";
    static rosidl_service_type_support_t test_handle = {
        .typesupport_identifier = test_identifier,
        .data = NULL,
        .func = NULL
    };
    
    // Call the API with matching identifier
    const rosidl_service_type_support_t *result1 = 
        get_service_typesupport_handle_function(&test_handle, test_identifier);
    
    // Call with non-matching identifier
    const char *non_matching_id = "rosidl_typesupport_other";
    const rosidl_service_type_support_t *result2 = 
        get_service_typesupport_handle_function(&test_handle, non_matching_id);
    
    // Use fuzz data to create test identifier
    if (size > 0) {
        // Create a null-terminated string from fuzz data (max 255 chars + null)
        char fuzz_identifier[256];
        size_t id_len = size < 255 ? size : 255;
        memcpy(fuzz_identifier, data, id_len);
        fuzz_identifier[id_len] = '\0';
        
        // Create another handle with fuzz identifier
        rosidl_service_type_support_t fuzz_handle = {
            .typesupport_identifier = fuzz_identifier,
            .data = NULL,
            .func = NULL
        };
        
        // Test with fuzz identifier
        const rosidl_service_type_support_t *result3 = 
            get_service_typesupport_handle_function(&fuzz_handle, fuzz_identifier);
        
        // Test with the original test identifier
        const rosidl_service_type_support_t *result4 = 
            get_service_typesupport_handle_function(&fuzz_handle, test_identifier);
        
        (void)result1; (void)result2; (void)result3; (void)result4; // Suppress unused warnings
    }
    
    // 5. Test rosidl_runtime_c__bool__Sequence__are_equal
    rosidl_runtime_c__boolean__Sequence bool_seq1, bool_seq2;
    
    if (init_bool_sequence_from_fuzz(&bool_seq1, data, size, size/2) &&
        init_bool_sequence_from_fuzz(&bool_seq2, data, size, size/3)) {
        
        // Call the API
        bool are_equal = rosidl_runtime_c__bool__Sequence__are_equal(&bool_seq1, &bool_seq2);
        
        // Test with NULL pointers
        bool null_test1 = rosidl_runtime_c__bool__Sequence__are_equal(NULL, &bool_seq2);
        bool null_test2 = rosidl_runtime_c__bool__Sequence__are_equal(&bool_seq1, NULL);
        bool null_test3 = rosidl_runtime_c__bool__Sequence__are_equal(NULL, NULL);
        
        // Test equality with itself
        bool self_equal = rosidl_runtime_c__bool__Sequence__are_equal(&bool_seq1, &bool_seq1);
        
        (void)are_equal; (void)null_test1; (void)null_test2; (void)null_test3; (void)self_equal;
        
        // Clean up
        rosidl_runtime_c__bool__Sequence__fini(&bool_seq1);
        rosidl_runtime_c__bool__Sequence__fini(&bool_seq2);
    }
    
    // Additional test: Create sequences of different sizes and test equality
    rosidl_runtime_c__boolean__Sequence small_seq, large_seq;
    if (rosidl_runtime_c__bool__Sequence__init(&small_seq, 2) &&
        rosidl_runtime_c__bool__Sequence__init(&large_seq, 5)) {
        
        // Initialize with some values
        if (small_seq.capacity >= 2) {
            small_seq.data[0] = true;
            small_seq.data[1] = false;
            small_seq.size = 2;
        }
        
        if (large_seq.capacity >= 5) {
            for (size_t i = 0; i < 5; i++) {
                large_seq.data[i] = (i % 2 == 0);
            }
            large_seq.size = 5;
        }
        
        // These should not be equal due to different sizes
        bool diff_size_equal = rosidl_runtime_c__bool__Sequence__are_equal(&small_seq, &large_seq);
        (void)diff_size_equal;
        
        rosidl_runtime_c__bool__Sequence__fini(&small_seq);
        rosidl_runtime_c__bool__Sequence__fini(&large_seq);
    }
    
    return 0;  // Non-zero return values are reserved for future use
}
