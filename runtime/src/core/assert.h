#pragma once
#include "defines.h"

// Implementation in logger
void _report_assertion_failure(const char* expression, const char* message, const char* file, u32 line);

#define debug_break() __asm__("int3");

#define ASSERT(expr) { \
  if (expr) { \
  } else { \
    _report_assertion_failure(#expr, "", __FILE__, __LINE__); \
    debug_break() \
  } \
} \

#define ASSERT_MSG(expr, message) { \
  if (expr) { \
  } else { \
    _report_assertion_failure(#expr, message, __FILE__, __LINE__); \
    debug_break \
  } \
} \
