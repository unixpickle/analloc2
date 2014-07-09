#ifndef __ANALLOC2_LOG_HPP__
#define __ANALLOC2_LOG_HPP__

#include "int.hpp"

namespace ANAlloc {

inline int Log2Floor(UInt value) {
  return sizeof(long long) * 8 - __builtin_clzll(value) - 1;
};

int Log2Ceil(UInt value);

}

#endif
