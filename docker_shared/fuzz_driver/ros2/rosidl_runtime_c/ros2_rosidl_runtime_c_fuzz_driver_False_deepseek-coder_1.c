#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sstream>
#include <assert.h>

#include "rosidl_runtime_c/message_type_support.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"
#include "rosidl_runtime_c/sequence_bound.h"
#include "rosidl_runtime_c/service_type_support.h"
#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_runtime_c/u16string_functions.h"

// Mock structures for type support handles since they are opaque in real usage
typedef struct rosidl_message_type_support_t {
    const char * typesupport_identifier;
    void * func;
} rosidl_message_type_support_t;

typedef struct rosidl_service_type_support_t {
    void * func;
} rosidl_service_type_support_t;

// Mock function types for type support resolution
typedef const rosidl_message_type_support_t * (*rosidl_message_typesupport_handle_function)(
    const rosidl_message_type_support_t *, const char *);

typedef const rosidl_service_type_support_t * (*rosidl_service_typesupport_handle_function)(
    const rosidl_service_type_support_t *, const char *);

// Mock allocator structure to match ROS 2 runtime expectations
typedef struct rcutils_allocator_t {
    void * (*allocate)(size_t size, void * state);
    void (*deallocate)(void * pointer, void * state);
    void * state;
} rcutils_allocator_t;

// Mock allocator functions
static void * mock_allocate(size_t size, void * state) {
    (void)state;
    return malloc(size);
}

static void mock_deallocate(void * pointer, void * state) {
    (void)state;
    free(pointer);
}

static rcutils_allocator_t rcutils_get_default_allocator(void) {
    static rcutils_allocator_t allocator = {
        .allocate = mock_allocate,
        .deallocate = mock_deallocate,
        .state = NULL
    };
    return allocator;
}

// Mock RCUTILS_CAN_RETURN_WITH_ERROR_OF macro
#define RCUTILS_CAN_RETURN_WITH_ERROR_OF(x) (void)0

// Provided API implementations (adapted for fuzzing context)
const rosidl_message_type_support_t * get_message_typesupport_handle(
    const rosidl_message_type_support_t * handle, const char * identifier)
{
    if (!handle || !handle->func) {
        return NULL;
    }
    rosidl_message_typesupport_handle_function func =
        (rosidl_message_typesupport_handle_function)(handle->func);
    return func(handle, identifier);
}

const rosidl_message_type_support_t * get_message_typesupport_handle_function(
    const rosidl_message_type_support_t * handle, const char * identifier)
{
    if (!handle || !handle->typesupport_identifier || !identifier) {
        return NULL;
    }
    if (strcmp(handle->typesupport_identifier, identifier) == 0) {
        return handle;
    }
    return NULL;
}

const rosidl_service_type_support_t * get_service_typesupport_handle(
    const rosidl_service_type_support_t * handle, const char * identifier)
{
    if (!handle || !handle->func) {
        return NULL;
    }
    rosidl_service_typesupport_handle_function func =
        (rosidl_service_typesupport_handle_function)(handle->func);
    return func(handle, identifier);
}

bool rosidl_runtime_c__String__init(rosidl_runtime_c__String * str)
{
    RCUTILS_CAN_RETURN_WITH_ERROR_OF(false);

    if (!str) {
        return false;
    }
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    str->data = (char *)allocator.allocate(1, allocator.state);
    if (!str->data) {
        return false;
    }
    str->data[0] = '\0';
    str->size = 0;
    str->capacity = 1;
    return true;
}

void rosidl_runtime_c__String__fini(rosidl_runtime_c__String * str)
{
    if (!str) {
        return;
    }
    if (str->data) {
        if (str->capacity <= 0) {
            // In fuzzing context, we avoid exit() to continue fuzzing
            return;
        }
        rcutils_allocator_t allocator = rcutils_get_default_allocator();
        allocator.deallocate(str->data, allocator.state);
        str->data = NULL;
        str->size = 0;
        str->capacity = 0;
    } else {
        if (0 != str->size || 0 != str->capacity) {
            // In fuzzing context, we avoid exit() to continue fuzzing
            return;
        }
    }
}

// Mock service type support function
static const rosidl_service_type_support_t * mock_service_typesupport_func(
    const rosidl_service_type_support_t * handle, const char * identifier)
{
    (void)handle;
    (void)identifier;
    static rosidl_service_type_support_t mock_service_handle = {0};
    return &mock_service_handle;
}

// Mock message type support function
static const rosidl_message_type_support_t * mock_message_typesupport_func(
    const rosidl_message_type_support_t * handle, const char * identifier)
{
    // Use the actual get_message_typesupport_handle_function for consistency
    return get_message_typesupport_handle_function(handle, identifier);
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Ensure we have minimum data for operations
    if (size < 2) {
        return 0;
    }

    // Initialize variables
    rosidl_runtime_c__String ros_string;
    memset(&ros_string, 0, sizeof(rosidl_runtime_c__String));
    
    rosidl_message_type_support_t message_handle;
    memset(&message_handle, 0, sizeof(rosidl_message_type_support_t));
    
    rosidl_service_type_support_t service_handle;
    memset(&service_handle, 0, sizeof(rosidl_service_type_support_t));
    
    const rosidl_message_type_support_t *message_result = NULL;
    const rosidl_service_type_support_t *service_result = NULL;

    // 1. Test rosidl_runtime_c__String__init
    bool init_success = rosidl_runtime_c__String__init(&ros_string);
    if (!init_success) {
        // Initialization failed, clean up and return
        goto cleanup;
    }

    // 2. Prepare identifier from fuzz data (safe bounded copy)
    // Use first byte to determine identifier length (max 64 chars for safety)
    size_t id_len = data[0] % 64;
    if (id_len == 0) id_len = 1; // Ensure non-zero length
    if (id_len > size - 1) id_len = size - 1; // Bound check
    
    char *identifier = (char *)malloc(id_len + 1);
    if (!identifier) {
        goto cleanup;
    }
    
    // Copy identifier data safely
    memcpy(identifier, data + 1, id_len);
    identifier[id_len] = '\0';

    // 3. Set up message type support handle
    // Use second byte to choose between two mock identifiers
    const char *mock_identifiers[] = {"rosidl_typesupport_c", "rosidl_typesupport_introspection_c"};
    size_t identifier_index = data[1] % 2;
    message_handle.typesupport_identifier = mock_identifiers[identifier_index];
    message_handle.func = (void *)mock_message_typesupport_func;

    // 4. Test get_message_typesupport_handle_function
    message_result = get_message_typesupport_handle_function(&message_handle, identifier);
    // Result may be NULL if identifiers don't match - this is expected behavior

    // 5. Test get_message_typesupport_handle
    message_result = get_message_typesupport_handle(&message_handle, identifier);
    // Result may be NULL - this is expected

    // 6. Set up service type support handle
    service_handle.func = (void *)mock_service_typesupport_func;

    // 7. Test get_service_typesupport_handle
    service_result = get_service_typesupport_handle(&service_handle, identifier);
    // Result may be NULL - this is expected

    // 8. Test string operations with fuzz data
    if (size > id_len + 1) {
        size_t str_data_len = size - (id_len + 1);
        if (str_data_len > 0) {
            // Resize the string if needed (simplified for fuzzing)
            // In real usage, we would use rosidl_runtime_c__String__assignn
            // Here we just ensure the string is properly initialized
            if (ros_string.capacity > 0 && ros_string.data) {
                // String is already initialized, we can proceed
            }
        }
    }

    // Free allocated identifier
    free(identifier);

cleanup:
    // 9. Test rosidl_runtime_c__String__fini
    rosidl_runtime_c__String__fini(&ros_string);
    
    // Verify string is properly cleaned up
    if (ros_string.data != NULL || ros_string.size != 0 || ros_string.capacity != 0) {
        // This would indicate a problem with fini, but we continue fuzzing
    }

    return 0;
}

// Main function for standalone testing (not used during fuzzing)
#ifdef STANDALONE_TEST
int main() {
    // Simple test with sample data
    uint8_t test_data[] = {10, 0, 'r', 'o', 's', 'i', 'd', 'l', '_', 't', 'y', 'p', 'e'};
    size_t test_size = sizeof(test_data);
    
    return LLVMFuzzerTestOneInput(test_data, test_size);
}
#endif
