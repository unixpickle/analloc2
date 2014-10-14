#ifndef __ANALLOC2_BITMAP_HPP__
#define __ANALLOC2_BITMAP_HPP__

#include <cstddef>
#include <cassert>
#include <ansa/math>

namespace analloc {

template <typename Unit = unsigned int, typename IndexType = size_t>
class Bitmap {
public:
  static constexpr IndexType UnitBitCount = ansa::NumericInfo<Unit>::bitCount;
  
  Bitmap(Unit * ptr, IndexType bc) : units(ptr), bitCount(bc) {
    assert(bc / ansa::NumericInfo<Unit>::bitCount <
           ansa::NumericInfo<size_t>::max);
  }
  
  Bitmap(Bitmap && bm) : units(bm.units), bitCount(bm.bitCount) {
    bm.units = nullptr;
  }
  
  Bitmap & operator=(Bitmap && bm) {
    units = bm.units;
    bitCount = bm.bitCount;
    bm.units = nullptr;
    return *this;
  }
  
  Bitmap(const Bitmap & bm) = delete;
  Bitmap & operator=(const Bitmap & bm) = delete;
  
  void SetBit(IndexType idx, bool value) {
    assert(idx < bitCount);
    IndexType unitIndex = idx / UnitBitCount;
    IndexType bitIndex = idx % UnitBitCount;
    if (value) {
      UnitAt(unitIndex) |= ((Unit)1 << bitIndex);
    } else {
      UnitAt(unitIndex) &= ~((Unit)1 << bitIndex);
    }
  }
  
  bool GetBit(IndexType idx) const {
    assert(idx < bitCount);
    IndexType unitIndex = idx / UnitBitCount;
    IndexType bitIndex = idx % UnitBitCount;
    return (UnitAt(unitIndex) & ((Unit)1 << bitIndex)) != 0;
  }
  
  template <typename ValueType>
  void SetBits(IndexType idx, IndexType len, ValueType value) {
    assert(!ansa::AddWraps<IndexType>(idx, len));
    assert(idx + len <= bitCount);
    for (IndexType i = 0; i < len; ++i) {
      bool flag = (value & ((ValueType)1 << i)) ? 1 : 0;
      SetBit(idx + len - (i + 1), flag);
    }
  }
  
  template <typename ValueType>
  ValueType GetBits(IndexType idx, IndexType len) {
    assert(!ansa::AddWraps<IndexType>(idx, len));
    assert(idx + len <= bitCount);
    ValueType result = 0;
    for (IndexType i = 0; i < len; ++i) {
      ValueType flag = (ValueType)(GetBit(idx + i) ? 1 : 0);
      result |= flag << (len - (i + 1));
    }
    return result;
  }
  
  inline IndexType GetBitCount() const {
    return bitCount;
  }
  
protected:
  Unit * units;
  IndexType bitCount;
  
  inline Unit & UnitAt(IndexType idx) {
    return units[(size_t)idx];
  }
  
  inline const Unit & UnitAt(IndexType idx) const {
    return units[(size_t)idx];
  }
};

}

#endif
