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

bool Bitmap::GetBit(uintptr_t idx) const {
  assert(idx < bitCount);
  
  uintptr_t byteIndex = idx >> 3;
  uintptr_t bitIndex = idx & 7;
  return (ptr[byteIndex] & (1 << bitIndex)) != 0;
}

}
