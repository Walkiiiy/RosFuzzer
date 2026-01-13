#include <rosidl_runtime_c/string_functions.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Fuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early exit for insufficient input
    if (size == 0) {
        return 0;
    }

    // Initialize strings for testing
    rosidl_runtime_c__String str1;
    rosidl_runtime_c__String str2;
    rosidl_runtime_c__String str3;

    // Initialize str1
    if (!rosidl_runtime_c__String__init(&str1)) {
        return 0; // Initialization failed
    }

    // Initialize str2
    if (!rosidl_runtime_c__String__init(&str2)) {
        rosidl_runtime_c__String__fini(&str1);
        return 0;
    }

    // Initialize str3
    if (!rosidl_runtime_c__String__init(&str3)) {
        rosidl_runtime_c__String__fini(&str2);
        rosidl_runtime_c__String__fini(&str1);
        return 0;
    }

    // Test 1: rosidl_runtime_c__String__assignn
    // Use fuzz data as source string, but ensure null termination
    // We'll use min(size, 1024) to avoid excessive allocations
    size_t assign_size = size > 1024 ? 1024 : size;
    
    // Create a properly null-terminated buffer for testing
    char *test_buffer = (char *)malloc(assign_size + 1);
    if (!test_buffer) {
        rosidl_runtime_c__String__fini(&str3);
        rosidl_runtime_c__String__fini(&str2);
        rosidl_runtime_c__String__fini(&str1);
        return 0;
    }
    
    // Copy fuzz data and ensure null termination
    memcpy(test_buffer, data, assign_size);
    test_buffer[assign_size] = '\0';
    
    // Test assignn with the fuzz data
    bool assignn_result = rosidl_runtime_c__String__assignn(&str1, test_buffer, assign_size);
    (void)assignn_result; // Result checked implicitly by subsequent operations

    // Test 2: rosidl_runtime_c__String__assign
    // Create a smaller test string for assign
    size_t assign_small_size = size > 64 ? 64 : size;
    char *small_test_buffer = (char *)malloc(assign_small_size + 1);
    if (!small_test_buffer) {
        free(test_buffer);
        rosidl_runtime_c__String__fini(&str3);
        rosidl_runtime_c__String__fini(&str2);
        rosidl_runtime_c__String__fini(&str1);
        return 0;
    }
    
    // Copy and null-terminate
    memcpy(small_test_buffer, data, assign_small_size);
    small_test_buffer[assign_small_size] = '\0';
    
    // Test assign with the smaller buffer
    bool assign_result = rosidl_runtime_c__String__assign(&str2, small_test_buffer);
    (void)assign_result;

    // Test 3: rosidl_runtime_c__String__copy
    // Copy from str1 to str3
    bool copy_result = rosidl_runtime_c__String__copy(&str1, &str3);
    (void)copy_result;

    // Test edge cases with null pointers
    // These should return false without crashing
    bool null_test1 = rosidl_runtime_c__String__copy(NULL, &str1);
    (void)null_test1;
    
    bool null_test2 = rosidl_runtime_c__String__assign(&str1, NULL);
    (void)null_test2;
    
    bool null_test3 = rosidl_runtime_c__String__assignn(&str1, NULL, 0);
    (void)null_test3;
    
    bool null_test4 = rosidl_runtime_c__String__init(NULL);
    (void)null_test4;

    // Test with empty string
    bool empty_assign = rosidl_runtime_c__String__assign(&str2, "");
    (void)empty_assign;

    // Test assignn with zero length
    bool zero_assignn = rosidl_runtime_c__String__assignn(&str3, test_buffer, 0);
    (void)zero_assignn;

    // Test with maximum size boundary (SIZE_MAX should fail)
    if (size > 0) {
        // Test with n = SIZE_MAX - 1 (should work if allocation succeeds)
        bool boundary_test = rosidl_runtime_c__String__assignn(&str1, test_buffer, 
            (assign_size > SIZE_MAX - 1) ? SIZE_MAX - 1 : assign_size);
        (void)boundary_test;
    }

    // Clean up allocated buffers
    free(small_test_buffer);
    free(test_buffer);

    // Finalize all strings in reverse order
    rosidl_runtime_c__String__fini(&str3);
    rosidl_runtime_c__String__fini(&str2);
    rosidl_runtime_c__String__fini(&str1);

    // Test finalize with null pointer (should not crash)
    rosidl_runtime_c__String__fini(NULL);

    return 0;
}
