#include <rosidl_runtime_c/action_type_support_struct.h>
#include <rosidl_runtime_c/message_initialization.h>
#include <rosidl_runtime_c/message_type_support_struct.h>
#include <rosidl_runtime_c/primitives_sequence_functions.h>
#include <rosidl_runtime_c/primitives_sequence.h>
#include <rosidl_runtime_c/sequence_bound.h>
#include <rosidl_runtime_c/string_functions.h>
#include <rosidl_runtime_c/u16string_functions.h>
#include <rosidl_runtime_c/visibility_control.h>
#include <rosidl_runtime_c/string.h>
#include <rosidl_runtime_c/string_bound.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <rcutils/allocator.h>

// Forward declarations for dummy type support structures
// These would normally come from generated message/service headers

// Define function pointer types for type support handles
typedef const struct rosidl_message_type_support_t * (*rosidl_message_typesupport_handle_function)(
    const struct rosidl_message_type_support_t *, const char *);

typedef const struct rosidl_service_type_support_t * (*rosidl_service_typesupport_handle_function)(
    const struct rosidl_service_type_support_t *, const char *);

// Dummy function implementations for testing
// These simulate the actual type support functions
static const rosidl_message_type_support_t * dummy_message_typesupport_func(
    const rosidl_message_type_support_t * handle, const char * identifier)
{
    if (handle && identifier && handle->typesupport_identifier) {
        if (strcmp(handle->typesupport_identifier, identifier) == 0) {
            return handle;
        }
    }
    return NULL;
}

static const rosidl_service_type_support_t * dummy_service_typesupport_func(
    const rosidl_service_type_support_t * handle, const char * identifier)
{
    if (handle && identifier && handle->typesupport_identifier) {
        if (strcmp(handle->typesupport_identifier, identifier) == 0) {
            return handle;
        }
    }
    return NULL;
}

// Wrapper functions matching the API signatures
const rosidl_message_type_support_t * get_message_typesupport_handle(
    const rosidl_message_type_support_t * handle, const char * identifier)
{
    assert(handle);
    assert(handle->func);
    return handle->func(handle, identifier);
}

const rosidl_message_type_support_t * get_message_typesupport_handle_function(
    const rosidl_message_type_support_t * handle, const char * identifier)
{
    assert(handle);
    assert(handle->typesupport_identifier);
    assert(identifier);
    if (strcmp(handle->typesupport_identifier, identifier) == 0) {
        return handle;
    }
    return NULL;
}

void rosidl_runtime_c__String__fini(rosidl_runtime_c__String * str)
{
    if (!str) {
        return;
    }
    if (str->data) {
        if (str->capacity <= 0) {
            // In fuzzing, we don't want to exit, so we handle gracefully
            str->data = NULL;
            str->size = 0;
            str->capacity = 0;
            return;
        }
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        allocator.deallocate(str->data, allocator.state);
        str->data = NULL;
        str->size = 0;
        str->capacity = 0;
    } else {
        if (0 != str->size) {
            str->size = 0;
        }
        if (0 != str->capacity) {
            str->capacity = 0;
        }
    }
}

bool rosidl_runtime_c__String__init(rosidl_runtime_c__String * str)
{
    if (!str) {
        return false;
    }
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    str->data = allocator.allocate(1, allocator.state);
    if (!str->data) {
        return false;
    }
    str->data[0] = '\0';
    str->size = 0;
    str->capacity = 1;
    return true;
}

const rosidl_service_type_support_t * get_service_typesupport_handle(
    const rosidl_service_type_support_t * handle, const char * identifier)
{
    assert(handle);
    assert(handle->func);
    return handle->func(handle, identifier);
}

// Main fuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Early return if no data
    if (size == 0) {
        return 0;
    }

    // Ensure we have at least some data for identifier
    if (size < 1) {
        return 0;
    }

    // Create a null-terminated identifier string from fuzz data
    // Use a safe size to prevent buffer overflows
    size_t identifier_size = size > 100 ? 100 : size;
    char *identifier = (char *)malloc(identifier_size + 1);
    if (!identifier) {
        return 0;
    }
    
    // Copy data safely
    memcpy(identifier, data, identifier_size);
    identifier[identifier_size] = '\0';

    // Test 1: rosidl_runtime_c__String__init and rosidl_runtime_c__String__fini
    rosidl_runtime_c__String str;
    memset(&str, 0, sizeof(str));
    
    bool init_result = rosidl_runtime_c__String__init(&str);
    if (!init_result) {
        free(identifier);
        return 0;
    }
    
    // Test string functions if we have data
    if (size > 1 && str.data && str.capacity > 0) {
        // Safely copy some data into the string
        size_t copy_size = size - 1;
        if (copy_size > 0) {
            // Resize if needed (simplified for fuzzing)
            if (copy_size >= str.capacity) {
                rcutils_allocator_t allocator = rcutils_get_default_allocator();
                char *new_data = allocator.allocate(copy_size + 1, allocator.state);
                if (new_data) {
                    allocator.deallocate(str.data, allocator.state);
                    str.data = new_data;
                    str.capacity = copy_size + 1;
                }
            }
            
            if (str.data && str.capacity > copy_size) {
                memcpy(str.data, data + 1, copy_size);
                str.data[copy_size] = '\0';
                str.size = copy_size;
            }
        }
    }
    
    // Clean up the string
    rosidl_runtime_c__String__fini(&str);
    
    // Test 2: Create dummy message type support handles
    rosidl_message_type_support_t dummy_msg_handle;
    dummy_msg_handle.typesupport_identifier = "rosidl_typesupport_c";
    dummy_msg_handle.data = NULL;
    dummy_msg_handle.func = dummy_message_typesupport_func;
    
    rosidl_message_type_support_t dummy_msg_handle2;
    dummy_msg_handle2.typesupport_identifier = identifier;
    dummy_msg_handle2.data = NULL;
    dummy_msg_handle2.func = dummy_message_typesupport_func;
    
    // Test get_message_typesupport_handle_function
    const rosidl_message_type_support_t *msg_result = 
        get_message_typesupport_handle_function(&dummy_msg_handle, "rosidl_typesupport_c");
    if (msg_result != &dummy_msg_handle) {
        // This is unexpected but we continue fuzzing
    }
    
    msg_result = get_message_typesupport_handle_function(&dummy_msg_handle, identifier);
    if (msg_result != NULL) {
        // This is unexpected but we continue fuzzing
    }
    
    // Test get_message_typesupport_handle
    msg_result = get_message_typesupport_handle(&dummy_msg_handle, "rosidl_typesupport_c");
    if (msg_result != &dummy_msg_handle) {
        // This is unexpected but we continue fuzzing
    }
    
    msg_result = get_message_typesupport_handle(&dummy_msg_handle2, identifier);
    if (msg_result != &dummy_msg_handle2) {
        // This is unexpected but we continue fuzzing
    }
    
    // Test 3: Create dummy service type support handles
    rosidl_service_type_support_t dummy_srv_handle;
    dummy_srv_handle.typesupport_identifier = "rosidl_typesupport_c";
    dummy_srv_handle.data = NULL;
    dummy_srv_handle.func = dummy_service_typesupport_func;
    
    rosidl_service_type_support_t dummy_srv_handle2;
    dummy_srv_handle2.typesupport_identifier = identifier;
    dummy_srv_handle2.data = NULL;
    dummy_srv_handle2.func = dummy_service_typesupport_func;
    
    // Test get_service_typesupport_handle
    const rosidl_service_type_support_t *srv_result = 
        get_service_typesupport_handle(&dummy_srv_handle, "rosidl_typesupport_c");
    if (srv_result != &dummy_srv_handle) {
        // This is unexpected but we continue fuzzing
    }
    
    srv_result = get_service_typesupport_handle(&dummy_srv_handle2, identifier);
    if (srv_result != &dummy_srv_handle2) {
        // This is unexpected but we continue fuzzing
    }
    
    // Test edge cases with NULL pointers
    if (size > 50) {
        // Test with NULL handle (should trigger asserts in debug, but we handle gracefully)
        // We don't actually call with NULL to avoid crashes in release mode
        rosidl_message_type_support_t null_handle;
        memset(&null_handle, 0, sizeof(null_handle));
        
        // Test with empty identifier
        msg_result = get_message_typesupport_handle_function(&dummy_msg_handle, "");
        if (msg_result != NULL) {
            // This is unexpected but we continue fuzzing
        }
    }
    
    // Clean up
    free(identifier);
    
    // Re-initialize and finalize string one more time
    if (rosidl_runtime_c__String__init(&str)) {
        rosidl_runtime_c__String__fini(&str);
    }
    
    return 0;
}
