#pragma once

#include "PyEngine/Core/Log.hpp"

#ifdef PYENGINE_ENABLE_ASSERTS
#define PYENGINE_ASSERT(condition, ...)                                        \
  do {                                                                         \
    if (!(condition)) {                                                        \
      PYENGINE_CORE_ERROR("Assertion failed: {}", #condition);                 \
      PYENGINE_CORE_ERROR(__VA_ARGS__);                                        \
      std::abort();                                                            \
    }                                                                          \
  } while (0)

#define PYENGINE_CORE_ASSERT(condition, ...)                                   \
  do {                                                                         \
    if (!(condition)) {                                                        \
      PYENGINE_CORE_ERROR("Core assertion failed: {}", #condition);            \
      PYENGINE_CORE_ERROR(__VA_ARGS__);                                        \
      std::abort();                                                            \
    }                                                                          \
  } while (0)
#else
#define PYENGINE_ASSERT(condition, ...)
#define PYENGINE_CORE_ASSERT(condition, ...)
#endif

#define PYENGINE_VERIFY(condition, ...)                                        \
  do {                                                                         \
    if (!(condition)) {                                                        \
      PYENGINE_CORE_ERROR("Verification failed: {}", #condition);              \
      PYENGINE_CORE_ERROR(__VA_ARGS__);                                        \
    }                                                                          \
  } while (0)
