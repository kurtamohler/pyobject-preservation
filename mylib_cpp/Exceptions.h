#pragma once

#include <stdexcept>

#define MYLIB_ASSERT(cond, message) \
  if (!(cond)) { \
    throw std::runtime_error(message); \
  }
