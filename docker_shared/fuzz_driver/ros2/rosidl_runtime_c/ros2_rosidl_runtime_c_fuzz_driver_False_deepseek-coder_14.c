#include <rosidl_runtime_c/message_type_support.h>
#include <rosidl_runtime_c/primitives_sequence_functions.h>
#include <rosidl_runtime_c/sequence_bound.h>
#include <rosidl_runtime_c/service_type_support.h>
#include <rosidl_runtime_c/string_functions.h>
#include <rosidl_runtime_c/u16string_functions.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

// LLVMFuzzerTestOneInput function signature
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early return if no data or insufficient size
    if (!data || size == 0) {
        return 0;
    }

    // Initialize ROS 2 string structures
    rosidl_runtime_c__String str1;
    rosidl_runtime_c__String str2;
    rosidl_runtime_c__String str3;
    
    // Initialize all strings to zero state
    memset(&str1, 0, sizeof(rosidl_runtime_c__String));
    memset(&str2, 0, sizeof(rosidl_runtime_c__String));
    memset(&str3, 0, sizeof(rosidl_runtime_c__String));

    // 1. Test rosidl_runtime_c__String__init
    if (!rosidl_runtime_c__String__init(&str1)) {
        // Initialization failed, clean up and return
        rosidl_runtime_c__String__fini(&str1);
        return 0;
    }

    // 2. Test rosidl_runtime_c__String__assignn
    // Use fuzz data as the source string for assignn
    // Ensure we don't exceed SIZE_MAX - 1
    size_t assign_size = size;
    if (assign_size == SIZE_MAX) {
        assign_size = SIZE_MAX - 1; // Prevent overflow in assignn
    }
    
    // Create a null-terminated copy of the fuzz data for safe usage
    char *fuzz_string = (char *)malloc(assign_size + 1);
    if (!fuzz_string) {
        rosidl_runtime_c__String__fini(&str1);
        return 0;
    }
    
    // Copy fuzz data and null-terminate
    memcpy(fuzz_string, data, assign_size);
    fuzz_string[assign_size] = '\0';
    
    // Test assignn with the fuzz data
    if (!rosidl_runtime_c__String__assignn(&str1, fuzz_string, assign_size)) {
        free(fuzz_string);
        rosidl_runtime_c__String__fini(&str1);
        return 0;
    }

    // 3. Test rosidl_runtime_c__String__init for second string
    if (!rosidl_runtime_c__String__init(&str2)) {
        free(fuzz_string);
        rosidl_runtime_c__String__fini(&str1);
        rosidl_runtime_c__String__fini(&str2);
        return 0;
    }

    // 4. Test rosidl_runtime_c__String__assign
    // Use a substring of fuzz data for testing assign
    size_t assign_len = (size > 10) ? 10 : size;
    char test_assign[11];
    if (assign_len > 0) {
        memcpy(test_assign, data, assign_len);
        test_assign[assign_len] = '\0';
        
        if (!rosidl_runtime_c__String__assign(&str2, test_assign)) {
            free(fuzz_string);
            rosidl_runtime_c__String__fini(&str1);
            rosidl_runtime_c__String__fini(&str2);
            return 0;
        }
    }

    // 5. Test rosidl_runtime_c__String__init for third string
    if (!rosidl_runtime_c__String__init(&str3)) {
        free(fuzz_string);
        rosidl_runtime_c__String__fini(&str1);
        rosidl_runtime_c__String__fini(&str2);
        rosidl_runtime_c__String__fini(&str3);
        return 0;
    }

    // 6. Test rosidl_runtime_c__String__copy
    // Copy from str1 to str3
    if (!rosidl_runtime_c__String__copy(&str1, &str3)) {
        free(fuzz_string);
        rosidl_runtime_c__String__fini(&str1);
        rosidl_runtime_c__String__fini(&str2);
        rosidl_runtime_c__String__fini(&str3);
        return 0;
    }

    // Verify the copy was successful by comparing sizes
    if (str1.size != str3.size) {
        // This shouldn't happen if copy succeeded, but we check anyway
        free(fuzz_string);
        rosidl_runtime_c__String__fini(&str1);
        rosidl_runtime_c__String__fini(&str2);
        rosidl_runtime_c__String__fini(&str3);
        return 0;
    }

    // Test edge cases with NULL pointers
    // These should fail gracefully
    rosidl_runtime_c__String__copy(NULL, &str3);  // Should return false
    rosidl_runtime_c__String__copy(&str1, NULL);  // Should return false
    rosidl_runtime_c__String__assign(NULL, "test");  // Should return false
    rosidl_runtime_c__String__assignn(NULL, "test", 4);  // Should return false
    rosidl_runtime_c__String__assign(&str2, NULL);  // Should return false
    rosidl_runtime_c__String__assignn(&str2, NULL, 0);  // Should return false

    // Test with empty string
    rosidl_runtime_c__String empty_str;
    memset(&empty_str, 0, sizeof(rosidl_runtime_c__String));
    if (rosidl_runtime_c__String__init(&empty_str)) {
        rosidl_runtime_c__String__assign(&empty_str, "");
        rosidl_runtime_c__String__fini(&empty_str);
    }

    // Clean up all allocated resources
    free(fuzz_string);
    rosidl_runtime_c__String__fini(&str1);
    rosidl_runtime_c__String__fini(&str2);
    rosidl_runtime_c__String__fini(&str3);

    // Return 0 to indicate successful execution
    return 0;
}
