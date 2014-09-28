#ifndef __ANALLOC2_BITMAP_HPP__
#define __ANALLOC2_BITMAP_HPP__

#include <cstddef>
#include <cassert>

namespace analloc {

template <typename Unit = unsigned int>
class Bitmap {
public:
  static const int UnitBitCount = sizeof(Unit) * 8;
  
  Bitmap() : units(NULL) {
  }
  
  Bitmap(Unit * ptr, size_t bc) : units(ptr), bitCount(bc) {
  }
  
  Bitmap(Bitmap && bm) : units(bm.units), bitCount(bm.bitCount) {
    bm.units = NULL;
  }
  
  Bitmap & operator=(Bitmap && bm) {
    units = bm.units;
    bitCount = bm.bitCount;
    bm.units = NULL;
    return *this;
  }
  
  Bitmap(const Bitmap & bm) = delete;
  Bitmap & operator=(const Bitmap & bm) = delete;
  
  void SetBit(size_t idx, bool value) {
    size_t unitIndex = idx / UnitBitCount;
    size_t bitIndex = idx % UnitBitCount;
    if (value) {
      units[unitIndex] |= ((Unit)1 << bitIndex);
    } else {
      units[unitIndex] &= ~((Unit)1 << bitIndex);
    }
  }
  
  bool GetBit(size_t idx) const {
    size_t unitIndex = idx / UnitBitCount;
    size_t bitIndex = idx % UnitBitCount;
    return (units[unitIndex] & ((Unit)1 << bitIndex)) != 0;
  }
  
  template <typename ValueType>
  void SetBits(size_t idx, size_t len, ValueType value) {
    assert(idx + len <= bitCount);
    for (size_t i = 0; i < len; ++i) {
      bool flag = (value & (1L << i)) ? 1 : 0;
      SetBit(idx + len - (i + 1), flag);
    }
  }
  
  template <typename ValueType>
  ValueType GetBits(size_t idx, size_t len) {
    ValueType result = 0;
    for (size_t i = 0; i < len; ++i) {
      ValueType flag = (ValueType)(GetBit(i + idx) ? 1 : 0);
      result |= flag << (len - (i + 1));
    }
    return result;
  }
  
  inline size_t GetBitCount() const {
    return bitCount;
  }
  
private:
  Unit * units;
  size_t bitCount;
};

}

#endif
