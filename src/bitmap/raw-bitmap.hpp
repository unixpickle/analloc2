#ifndef __ANALLOC2_RAW_BITMAP_HPP__
#define __ANALLOC2_RAW_BITMAP_HPP__

#include <cstddef>
#include <cassert>
#include <ansa/math>

namespace analloc {

/**
 * A [RawBitmap] facilitates bit-by-bit manipulation of a chunk of memory.
 *
 * The optional [IndexType] template argument will be used for bit indexes
 * throughout the bitmap.
 *
 * The [Unit] template argument determines the array type used by the backing
 * store of the bitmap.  Using a larger [Unit] may increase performance,
 * especially when subclasses can use unit-by-unit heuristics.
 */
template <typename Unit = unsigned int, typename IndexType = size_t>
class RawBitmap {
public:
  static constexpr IndexType UnitBitCount =
      (IndexType)ansa::NumericInfo<Unit>::bitCount;
  
  /**
   * Create a new [RawBitmap] with a region of memory [ptr] and a bit count
   * [bc].
   */
  RawBitmap(Unit * ptr, IndexType bc) : units(ptr), bitCount(bc) {
    assert(bc / ansa::NumericInfo<Unit>::bitCount <
           ansa::NumericInfo<size_t>::max);
  }
  
  /**
   * Move a [RawBitmap] [bm] into a new bitmap.
   */
  RawBitmap(RawBitmap && bm) : units(bm.units), bitCount(bm.bitCount) {
    bm.units = nullptr;
  }
  
  /**
   * Move a [RawBitmap] [bm] into this bitmap.
   */
  RawBitmap & operator=(RawBitmap && bm) {
    units = bm.units;
    bitCount = bm.bitCount;
    bm.units = nullptr;
    return *this;
  }
  
  RawBitmap(const RawBitmap & bm) = delete;
  RawBitmap & operator=(const RawBitmap & bm) = delete;
  
  /**
   * Set the bit at a given index [idx] to [value].
   */
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
  
  /**
   * Get the bit at a given index [idx].
   */
  bool GetBit(IndexType idx) const {
    assert(idx < bitCount);
    IndexType unitIndex = idx / UnitBitCount;
    IndexType bitIndex = idx % UnitBitCount;
    return (UnitAt(unitIndex) & ((Unit)1 << bitIndex)) != 0;
  }
  
  /**
   * Set a series of [len] bits starting at [idx], given the mask [value].
   */
  template <typename ValueType>
  void SetBits(IndexType idx, IndexType len, ValueType value) {
    assert(!ansa::AddWraps<IndexType>(idx, len));
    assert(idx + len <= bitCount);
    for (IndexType i = 0; i < len; ++i) {
      bool flag = (value & ((ValueType)1 << i)) ? 1 : 0;
      SetBit(idx + len - (i + 1), flag);
    }
  }
  
  /**
   * Read a series of [len] bits starting at [idx].
   */
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
  
  /**
   * Get the number of bits in this bitmap.
   */
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
