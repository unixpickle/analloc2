#include "bitmap.hpp"
#include <cstddef>
#include <cassert>

namespace ANAlloc {

Bitmap::Bitmap() {
  ptr = NULL;
  bitCount = 0;
}

Bitmap::Bitmap(const Bitmap & bm) {
  *this = bm;
}

Bitmap & Bitmap::operator=(const Bitmap & bm) {
  ptr = bm.ptr;
  bitCount = bm.bitCount;
  return *this;
}

Bitmap::Bitmap(uint8_t * _ptr, uint64_t count) {
  ptr = _ptr;
  bitCount = count;
}

void Bitmap::SetBit(uint64_t idx, bool value) {
  assert(idx < bitCount);
  
  uint64_t byteIndex = idx >> 3;
  uint64_t bitIndex = idx & 7;
  if (!value) {
    ptr[byteIndex] &= 0xff ^ (1 << bitIndex);
  } else {
    ptr[byteIndex] |= 1 << bitIndex;
  }
}

void Bitmap::SetMultibit(uint64_t idx, int len, uint64_t value) {
  assert(idx + len <= bitCount);
  for (int i = 0; i < len; i++) {
    bool flag = value & (1L << i) ? 1 : 0;
    SetBit(idx + len - i - 1, flag);
  }
}

bool Bitmap::GetBit(uint64_t idx) const {
  assert(idx < bitCount);
  
  uint64_t byteIndex = idx >> 3;
  uint64_t bitIndex = idx & 7;
  return (ptr[byteIndex] & (1 << bitIndex)) != 0;
}

uint64_t Bitmap::GetMultibit(uint64_t idx, int len) const {
  // there are len bits, and the one at idx is the most significant
  assert(idx + len <= bitCount);
  uint64_t result = 0;
  for (int i = 0; i < len; i++) {
    result += (uint64_t)GetBit(i + idx) << (len - i - 1);
  }
  return result;
}

uint64_t Bitmap::GetBitCount() const {
  return bitCount;
}


}
