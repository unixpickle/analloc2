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

Bitmap::Bitmap(uint8_t * _ptr, UInt count) {
  ptr = _ptr;
  bitCount = count;
}

void Bitmap::SetBit(UInt idx, bool value) {
  assert(idx < bitCount);
  
  UInt byteIndex = idx >> 3;
  UInt bitIndex = idx & 7;
  if (!value) {
    ptr[byteIndex] &= 0xff ^ (1 << bitIndex);
  } else {
    ptr[byteIndex] |= 1 << bitIndex;
  }
}

void Bitmap::SetMultibit(UInt idx, int len, UInt value) {
  assert(idx + len <= bitCount);
  for (int i = 0; i < len; i++) {
    bool flag = value & (1L << i) ? 1 : 0;
    SetBit(idx + len - i - 1, flag);
  }
}

bool Bitmap::GetBit(UInt idx) const {
  assert(idx < bitCount);
  
  UInt byteIndex = idx >> 3;
  UInt bitIndex = idx & 7;
  return (ptr[byteIndex] & (1 << bitIndex)) != 0;
}

UInt Bitmap::GetMultibit(UInt idx, int len) const {
  // there are len bits, and the one at idx is the most significant
  assert(idx + len <= bitCount);
  UInt result = 0;
  for (int i = 0; i < len; i++) {
    result += (UInt)GetBit(i + idx) << (len - i - 1);
  }
  return result;
}

UInt Bitmap::GetBitCount() const {
  return bitCount;
}


}
