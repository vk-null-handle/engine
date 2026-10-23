#pragma once
#include "defines.h"
#include "core/assert.h"
#include <vulkan/vk_enum_string_helper.h>

#define VK_CHECK(expr) { \
  if (expr == VK_SUCCESS) { \
  } else { \
    _report_assertion_failure(#expr, string_VkResult(expr), __FILE__, __LINE__); \
    debug_break() \
  } \
} \
