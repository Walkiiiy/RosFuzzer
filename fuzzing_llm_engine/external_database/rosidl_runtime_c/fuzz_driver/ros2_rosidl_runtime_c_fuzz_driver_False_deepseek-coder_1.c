#include <rosidl_runtime_c/string_functions.h>
#include <rosidl_runtime_c/u16string_functions.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

static size_t clamp_len(size_t len, size_t max_len) {
    return len < max_len ? len : max_len;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (!data || size == 0) {
        return 0;
    }

    const size_t max_str_len = 256;
    size_t len = clamp_len(size, max_str_len);

    // Prepare an ASCII-ish buffer (replace zeros to keep it valid for assign)
    char buf[257];
    size_t buf_len = len;
    if (buf_len > sizeof(buf) - 1) {
        buf_len = sizeof(buf) - 1;
    }
    for (size_t i = 0; i < buf_len; ++i) {
        buf[i] = (char)(data[i] ? data[i] : 'A');
    }
    buf[buf_len] = '\0';

    rosidl_runtime_c__String s1;
    rosidl_runtime_c__String s2;
    rosidl_runtime_c__String__init(&s1);
    rosidl_runtime_c__String__init(&s2);

    // assign / assignn
    rosidl_runtime_c__String__assignn(&s1, buf, buf_len);
    if (buf_len > 4) {
        rosidl_runtime_c__String__assignn(&s2, buf, buf_len / 2);
    } else {
        rosidl_runtime_c__String__assign(&s2, buf);
    }

    // resize and fill with a small pattern
    size_t new_len = (buf_len / 3) + 8;
    char resize_fill[257];
    size_t resize_fill_len = new_len < sizeof(resize_fill) - 1 ? new_len : sizeof(resize_fill) - 1;
    memset(resize_fill, 'Z', resize_fill_len);
    resize_fill[resize_fill_len] = '\0';
    rosidl_runtime_c__String__assignn(&s1, resize_fill, resize_fill_len);

    // copy / are_equal / compare
    rosidl_runtime_c__String__copy(&s1, &s2);
    (void)rosidl_runtime_c__String__are_equal(&s1, &s2);

    // sequence coverage
    rosidl_runtime_c__String__Sequence seq_a;
    rosidl_runtime_c__String__Sequence seq_b;
    size_t seq_len = (size % 5) + 1;
    rosidl_runtime_c__String__Sequence__init(&seq_a, seq_len);
    rosidl_runtime_c__String__Sequence__init(&seq_b, seq_len);
    for (size_t i = 0; i < seq_len; ++i) {
        rosidl_runtime_c__String__assignn(&seq_a.data[i], buf, clamp_len(buf_len, 16));
        rosidl_runtime_c__String__assign(&seq_b.data[i], buf);
    }
    (void)rosidl_runtime_c__String__Sequence__are_equal(&seq_a, &seq_b);
    rosidl_runtime_c__String__Sequence__copy(&seq_a, &seq_b);
    rosidl_runtime_c__String__Sequence__fini(&seq_a);
    rosidl_runtime_c__String__Sequence__fini(&seq_b);

    // u16string minimal coverage
    rosidl_runtime_c__U16String u16s;
    rosidl_runtime_c__U16String__init(&u16s);
    uint16_t u16buf[32];
    size_t u16len = clamp_len(size, 16);
    for (size_t i = 0; i < u16len; ++i) {
        u16buf[i] = (uint16_t)data[i];
    }
    rosidl_runtime_c__U16String__assignn(&u16s, u16buf, u16len);
    rosidl_runtime_c__U16String__resize(&u16s, u16len + 2);
    rosidl_runtime_c__U16String__fini(&u16s);

    rosidl_runtime_c__String__fini(&s1);
    rosidl_runtime_c__String__fini(&s2);
    return 0;
}
