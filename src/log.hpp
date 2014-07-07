#ifndef __ANALLOC2_LOG_HPP__
#define __ANALLOC2_LOG_HPP__

#include <cstdint>

namespace ANAlloc {

inline uint8_t Log2Floor(uint64_t value) {
  return sizeof(long long) * 8 - __builtin_clzll(value) - 1;
}

uint8_t Log2Ceil(uint64_t value);

}

#endif
