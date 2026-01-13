#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

/* ROS 2 headers */
#include "rosidl_runtime_c/action_type_support_struct.h"
#include "rosidl_runtime_c/message_initialization.h"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"
#include "rosidl_runtime_c/primitives_sequence.h"
#include "rosidl_runtime_c/sequence_bound.h"
#include "rosidl_runtime_c/service_type_support_struct.h"
#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_runtime_c/u16string_functions.h"
#include "rosidl_runtime_c/visibility_control.h"
#include "rosidl_runtime_c/string.h"
#include "rosidl_runtime_c/string_bound.h"

/* For rcutils allocator */
#include "rcutils/allocator.h"

/* For boolean sequence type */
#include "rosidl_runtime_c/primitives_sequence_functions.h"

/* Helper to extract values from fuzz input */
static uint8_t get_byte(const uint8_t *data, size_t size, size_t *offset) {
    if (*offset >= size) {
        return 0;
    }
    return data[(*offset)++];
}

static uint32_t get_uint32(const uint8_t *data, size_t size, size_t *offset) {
    uint32_t value = 0;
    for (int i = 0; i < 4; i++) {
        value = (value << 8) | get_byte(data, size, offset);
    }
    return value;
}

/* Create a simple handler function outside of LLVMFuzzerTestOneInput */
static rosidl_service_type_support_t* dummy_handler(
    const rosidl_service_type_support_t *handle, const char *identifier) {
    (void)handle;
    (void)identifier;
    static const rosidl_service_type_support_t dummy_return_handle = {0};
    return (rosidl_service_type_support_t*)&dummy_return_handle;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    /* Initialize offset for reading from fuzz input */
    size_t offset = 0;
    
    /* 1. Test rosidl_runtime_c__String__Sequence__init */
    rosidl_runtime_c__String__Sequence str_seq1 = {0};
    rosidl_runtime_c__String__Sequence str_seq2 = {0};
    
    /* Get sequence size from fuzz input (limit to reasonable size) */
    uint32_t seq_size = get_uint32(data, size, &offset);
    seq_size = seq_size % 256; /* Limit to prevent excessive memory usage */
    
    bool init_success1 = rosidl_runtime_c__String__Sequence__init(&str_seq1, seq_size);
    bool init_success2 = rosidl_runtime_c__String__Sequence__init(&str_seq2, seq_size);
    
    /* If initialization failed, clean up and return */
    if (!init_success1 || !init_success2) {
        if (init_success1) {
            rosidl_runtime_c__String__Sequence__fini(&str_seq1);
        }
        if (init_success2) {
            rosidl_runtime_c__String__Sequence__fini(&str_seq2);
        }
        return 0;
    }
    
    /* Fill string sequences with data from fuzz input */
    for (size_t i = 0; i < seq_size && offset < size; i++) {
        /* Get string length from fuzz input */
        uint8_t str_len = get_byte(data, size, &offset);
        str_len = str_len % 64; /* Limit string length */
        
        if (str_len > 0 && offset + str_len <= size) {
            /* Copy data to string */
            rosidl_runtime_c__String__assignn(&str_seq1.data[i], 
                                            (const char*)&data[offset], 
                                            str_len);
            rosidl_runtime_c__String__assignn(&str_seq2.data[i], 
                                            (const char*)&data[offset], 
                                            str_len);
            offset += str_len;
        }
    }
    
    /* 2. Test rosidl_runtime_c__String__Sequence__are_equal */
    bool strings_equal = rosidl_runtime_c__String__Sequence__are_equal(&str_seq1, &str_seq2);
    /* Should be true since we filled them with the same data */
    (void)strings_equal; /* Mark as used to avoid compiler warning */
    
    /* 3. Test rosidl_runtime_c__bool__Sequence__are_equal */
    rosidl_runtime_c__boolean__Sequence bool_seq1 = {0};
    rosidl_runtime_c__boolean__Sequence bool_seq2 = {0};
    
    /* Initialize boolean sequences */
    if (rosidl_runtime_c__boolean__Sequence__init(&bool_seq1, seq_size) &&
        rosidl_runtime_c__boolean__Sequence__init(&bool_seq2, seq_size)) {
        
        /* Fill boolean sequences with data from fuzz input */
        for (size_t i = 0; i < seq_size && offset < size; i++) {
            bool_seq1.data[i] = (get_byte(data, size, &offset) & 0x01) != 0;
            bool_seq2.data[i] = bool_seq1.data[i]; /* Make them equal */
        }
        
        /* Test equality */
        bool bools_equal = rosidl_runtime_c__bool__Sequence__are_equal(&bool_seq1, &bool_seq2);
        (void)bools_equal; /* Mark as used */
        
        /* Clean up boolean sequences */
        rosidl_runtime_c__boolean__Sequence__fini(&bool_seq1);
        rosidl_runtime_c__boolean__Sequence__fini(&bool_seq2);
    }
    
    /* 4. Test get_message_typesupport_handle_function */
    /* Create a dummy message typesupport structure */
    static const char dummy_identifier[] = "rosidl_typesupport_c";
    static rosidl_message_type_support_t dummy_handle = {
        .typesupport_identifier = dummy_identifier,
        .data = NULL,
        .func = NULL
    };
    
    /* Test with matching identifier */
    const rosidl_message_type_support_t *msg_handle = 
        get_message_typesupport_handle_function(&dummy_handle, dummy_identifier);
    (void)msg_handle; /* Mark as used */
    
    /* Test with non-matching identifier */
    const char *non_matching = "rosidl_typesupport_other";
    msg_handle = get_message_typesupport_handle_function(&dummy_handle, non_matching);
    (void)msg_handle; /* Mark as used */
    
    /* 5. Test get_service_typesupport_handle */
    /* Create a dummy service typesupport structure with a simple function */
    static rosidl_service_type_support_t dummy_service_handle = {0};
    
    dummy_service_handle.func = (const void*)dummy_handler;
    
    /* Call get_service_typesupport_handle */
    const rosidl_service_type_support_t *service_handle = 
        get_service_typesupport_handle(&dummy_service_handle, dummy_identifier);
    (void)service_handle; /* Mark as used */
    
    /* Clean up string sequences */
    rosidl_runtime_c__String__Sequence__fini(&str_seq1);
    rosidl_runtime_c__String__Sequence__fini(&str_seq2);
    
    return 0;
}
