#ifndef __ANALLOC2_UTILITY_H__
#define __ANALLOC2_UTILITY_H__

namespace ANAlloc {

inline int Log2Floor(uintptr_t num) {
  assert(num != 0);
  if (1 == num) return 0;
  for (int i = 0; i < 64; i++) {
    uintptr_t val = (1L << i);
    if (val > num) return i - 1;
    if (val == num) return i;
  }
  return 63;
}

inline int Log2Ceil(uintptr_t num) {
  assert(num != 0);
  if (1 == num) return 0;
  for (int i = 0; i < 64; i++) {
    uintptr_t val = (1L << i);
    if (val >= num) return i;
  }
  return 64;
}

}

#endif
