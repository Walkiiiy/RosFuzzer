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

// Forward declarations for types used in the API
typedef struct rosidl_runtime_c__boolean__Sequence
{
    bool * data;
    size_t size;
    size_t capacity;
} rosidl_runtime_c__boolean__Sequence;

typedef struct rosidl_runtime_c__String
{
    char * data;
    size_t size;
    size_t capacity;
} rosidl_runtime_c__String;

typedef struct rosidl_runtime_c__String__Sequence
{
    rosidl_runtime_c__String * data;
    size_t size;
    size_t capacity;
} rosidl_runtime_c__String__Sequence;

// Function prototypes from the provided API
bool rosidl_runtime_c__bool__Sequence__are_equal(
    const rosidl_runtime_c__boolean__Sequence * lhs,
    const rosidl_runtime_c__boolean__Sequence * rhs);
bool rosidl_runtime_c__String__init(rosidl_runtime_c__String * str);
bool rosidl_runtime_c__String__Sequence__init(
    rosidl_runtime_c__String__Sequence * sequence, size_t size);
void rosidl_runtime_c__String__Sequence__fini(
    rosidl_runtime_c__String__Sequence * sequence);
void rosidl_runtime_c__String__fini(rosidl_runtime_c__String * str);

// Helper function to create boolean sequences from fuzz data
static void create_bool_sequence_from_fuzz(
    const uint8_t *data,
    size_t size,
    rosidl_runtime_c__boolean__Sequence *seq)
{
    if (size == 0) {
        seq->data = NULL;
        seq->size = 0;
        seq->capacity = 0;
        return;
    }
    
    // Use first byte to determine sequence size (modulo to limit size)
    size_t seq_size = data[0] % 256;
    if (seq_size > size - 1) {
        seq_size = size > 1 ? size - 1 : 0;
    }
    
    if (seq_size == 0) {
        seq->data = NULL;
        seq->size = 0;
        seq->capacity = 0;
        return;
    }
    
    seq->data = (bool*)malloc(seq_size * sizeof(bool));
    if (!seq->data) {
        seq->size = 0;
        seq->capacity = 0;
        return;
    }
    
    seq->size = seq_size;
    seq->capacity = seq_size;
    
    // Fill with fuzz data (convert bytes to bool)
    for (size_t i = 0; i < seq_size && i + 1 < size; i++) {
        seq->data[i] = (data[i + 1] & 0x01) != 0;
    }
}

// Helper function to clean up boolean sequences
static void cleanup_bool_sequence(rosidl_runtime_c__boolean__Sequence *seq)
{
    if (seq && seq->data) {
        free(seq->data);
        seq->data = NULL;
        seq->size = 0;
        seq->capacity = 0;
    }
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early return if no data
    if (size == 0 || data == NULL) {
        return 0;
    }

    // Test 1: rosidl_runtime_c__String__init and rosidl_runtime_c__String__fini
    rosidl_runtime_c__String str1;
    memset(&str1, 0, sizeof(str1));
    
    if (!rosidl_runtime_c__String__init(&str1)) {
        // Initialization failed, nothing to clean up
        return 0;
    }
    
    // Clean up the string
    rosidl_runtime_c__String__fini(&str1);
    
    // Test 2: rosidl_runtime_c__String__Sequence__init and rosidl_runtime_c__String__Sequence__fini
    rosidl_runtime_c__String__Sequence str_seq;
    memset(&str_seq, 0, sizeof(str_seq));
    
    // Determine sequence size from fuzz data (limit to reasonable size)
    size_t seq_size = size > 0 ? (data[0] % 16) : 0;
    
    if (!rosidl_runtime_c__String__Sequence__init(&str_seq, seq_size)) {
        // Sequence initialization failed
        return 0;
    }
    
    // Clean up the string sequence
    rosidl_runtime_c__String__Sequence__fini(&str_seq);
    
    // Test 3: rosidl_runtime_c__bool__Sequence__are_equal
    rosidl_runtime_c__boolean__Sequence bool_seq1;
    rosidl_runtime_c__boolean__Sequence bool_seq2;
    
    memset(&bool_seq1, 0, sizeof(bool_seq1));
    memset(&bool_seq2, 0, sizeof(bool_seq2));
    
    // Create two boolean sequences from fuzz data
    create_bool_sequence_from_fuzz(data, size, &bool_seq1);
    
    // Use different offset for second sequence if enough data
    size_t offset = size > 10 ? 5 : 0;
    create_bool_sequence_from_fuzz(data + offset, 
                                  size > offset ? size - offset : 0, 
                                  &bool_seq2);
    
    // Compare the sequences
    bool are_equal = rosidl_runtime_c__bool__Sequence__are_equal(&bool_seq1, &bool_seq2);
    
    // Clean up boolean sequences
    cleanup_bool_sequence(&bool_seq1);
    cleanup_bool_sequence(&bool_seq2);
    
    // Test 4: Additional string initialization with actual data
    if (size > 1) {
        rosidl_runtime_c__String str2;
        memset(&str2, 0, sizeof(str2));
        
        if (rosidl_runtime_c__String__init(&str2)) {
            // Note: In real usage, we would copy data into the string
            // but the API doesn't provide a setter function in our provided set
            rosidl_runtime_c__String__fini(&str2);
        }
    }
    
    // Test 5: String sequence with non-zero size and individual string operations
    if (size > 2) {
        size_t test_seq_size = (data[1] % 8) + 1; // 1-8 elements
        rosidl_runtime_c__String__Sequence test_seq;
        memset(&test_seq, 0, sizeof(test_seq));
        
        if (rosidl_runtime_c__String__Sequence__init(&test_seq, test_seq_size)) {
            // Initialize and finalize individual strings in the sequence
            for (size_t i = 0; i < test_seq.size && i < 3; i++) {
                // Re-initialize a string (simulating modification)
                rosidl_runtime_c__String__fini(&test_seq.data[i]);
                rosidl_runtime_c__String__init(&test_seq.data[i]);
            }
            
            rosidl_runtime_c__String__Sequence__fini(&test_seq);
        }
    }

    return 0;
}
