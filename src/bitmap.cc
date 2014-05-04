#include "bitmap.h"

namespace ANAlloc {

Bitmap::Bitmap(uint8_t * _ptr, uintptr_t count) {
  ptr = _ptr;
  bitCount = count;
  for (uintptr_t i = 0; i < count / 8 + (count & 7 ? 1 : 0); i++) {
    ptr[i] = 0;
  }
}

void Bitmap::SetBit(uintptr_t idx, bool value) {
  assert(idx < bitCount);
  
  uintptr_t byteIndex = idx >> 3;
  uintptr_t bitIndex = idx & 7;
  if (!value) {
    ptr[byteIndex] &= 0xff ^ (1 << bitIndex);
  } else {
    ptr[byteIndex] |= 1 << bitIndex;
  }
}

void Bitmap::SetMultibit(uintptr_t idx, int len, uintptr_t value) {
  // TODO: optimize this
  assert(idx + len <= bitCount);
  for (int i = 0; i < len; i++) {
    bool flag = value & (1L << i) ? 1 : 0;
    SetBit(idx + len - i - 1, flag);
  }
}

bool Bitmap::GetBit(uintptr_t idx) const {
  assert(idx < bitCount);
  
  uintptr_t byteIndex = idx >> 3;
  uintptr_t bitIndex = idx & 7;
  return (ptr[byteIndex] & (1 << bitIndex)) != 0;
}

uintptr_t Bitmap::GetMultibit(uintptr_t idx, int len) const {
  // there are len bits, and the one at idx is the most significant
  // TODO: figure out if this can be optimized
  assert(idx + len <= bitCount);
  uintptr_t result = 0;
  for (int i = 0; i < len; i++) {
    result += (uintptr_t)GetBit(i + idx) << (len - i - 1);
  }
  return result;
}

uintptr_t Bitmap::GetBitCount() const {
  return bitCount;
}


}
