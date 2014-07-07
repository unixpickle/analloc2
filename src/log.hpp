#ifndef __ANALLOC2_LOG_HPP__
#define __ANALLOC2_LOG_HPP__

namespace ANAlloc {

inline uint8_t Log2Floor(uint64_t value) {
  return sizeof(long long) * 8 - __builtin_clzll(value) - 1;
}

uint8_t Log2Ceil(uint64_t value) {
  uint8_t floored = Log2Floor(value);
  if ((1UL << floored) != value) {
    return floored + 1;
  } else {
    return floored;
  }
}

}

#endif
