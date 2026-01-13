#include <stdbool.h>
#include <stddef.h>
#include "rosidl_runtime_c/string.h"
#include "rosidl_runtime_c/string_functions.h"

bool fuzz_function(rosidl_runtime_c__String__Sequence *seq, size_t i) {
    if (!rosidl_runtime_c__String__init(&seq->data[i])) {
        // Clean up any strings that were already initialized
        for (size_t j = 0; j < i; ++j) {
            rosidl_runtime_c__String__fini(&seq->data[j]);
        }
        // Use the proper sequence finalization function instead of manual cleanup
        rosidl_runtime_c__String__Sequence__fini(seq);
        return false;
    }
    return true;
}

// LibFuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < sizeof(size_t)) {
        return 0;
    }
    
    // Extract a size_t value from the fuzzer input
    size_t index = 0;
    for (size_t i = 0; i < sizeof(size_t) && i < size; i++) {
        index |= (size_t)data[i] << (8 * i);
    }
    
    // Create a sequence with a small fixed capacity for fuzzing
    rosidl_runtime_c__String__Sequence seq;
    seq.capacity = 10;
    seq.size = 10;
    seq.data = (rosidl_runtime_c__String*)malloc(sizeof(rosidl_runtime_c__String) * 10);
    
    if (!seq.data) {
        return 0;
    }
    
    // Initialize all strings in the sequence
    for (size_t i = 0; i < 10; i++) {
        if (!rosidl_runtime_c__String__init(&seq.data[i])) {
            // Clean up on failure
            for (size_t j = 0; j < i; j++) {
                rosidl_runtime_c__String__fini(&seq.data[j]);
            }
            free(seq.data);
            return 0;
        }
    }
    
    // Call the original fuzz_function with the fuzzer-provided index
    bool result = fuzz_function(&seq, index % 10);
    
    // Clean up
    for (size_t i = 0; i < 10; i++) {
        rosidl_runtime_c__String__fini(&seq.data[i]);
    }
    free(seq.data);
    
    return 0;
}
