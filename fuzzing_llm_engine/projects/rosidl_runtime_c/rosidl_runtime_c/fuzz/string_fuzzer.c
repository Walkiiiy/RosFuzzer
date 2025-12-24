#include <stddef.h>
#include <stdint.h>

#include "rosidl_runtime_c/string_functions.h"

// Simple libFuzzer target covering String init/assign/copy and sequence usage.
// It deliberately bounds allocations to avoid quadratic blowups while still
// exercising edge conditions around sizes and null handling.
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  rosidl_runtime_c__String str_a;
  rosidl_runtime_c__String str_b;
  if (!rosidl_runtime_c__String__init(&str_a)) {
    return 0;
  }
  if (!rosidl_runtime_c__String__init(&str_b)) {
    rosidl_runtime_c__String__fini(&str_a);
    return 0;
  }

  // Use the fuzz buffer directly; cap length to a reasonable bound.
  size_t take = size > 1024 ? 1024 : size;
  if (take > 0) {
    rosidl_runtime_c__String__assignn(&str_a, (const char *)data, take);
    // Exercise copy/compare paths.
    rosidl_runtime_c__String__copy(&str_a, &str_b);
    rosidl_runtime_c__String__are_equal(&str_a, &str_b);
  }

  // Build a small sequence of strings using chunks of the input.
  size_t seq_len = (size > 0) ? (data[size - 1] % 4) : 0;  // limit to 0-3
  rosidl_runtime_c__String__Sequence seq;
  if (rosidl_runtime_c__String__Sequence__init(&seq, seq_len)) {
    const uint8_t *cursor = data;
    size_t remaining = size;
    for (size_t i = 0; i < seq_len; ++i) {
      size_t chunk = remaining > 16 ? remaining % 16 : remaining;
      rosidl_runtime_c__String__assignn(
        &seq.data[i],
        (const char *)cursor,
        chunk);
      // Advance cursor safely.
      if (chunk > remaining) {
        chunk = remaining;
      }
      cursor += chunk;
      remaining -= chunk;
    }
    rosidl_runtime_c__String__Sequence__fini(&seq);
  }

  rosidl_runtime_c__String__fini(&str_b);
  rosidl_runtime_c__String__fini(&str_a);
  return 0;
}
