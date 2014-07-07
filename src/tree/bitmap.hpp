#ifndef __ANALLOC2_BITMAP_H__
#define __ANALLOC2_BITMAP_H__

#include <cstdint>

namespace ANAlloc {

/**
 * This stores an optimal representation of a bitmap.
 */
class Bitmap {
protected:
  uint8_t * ptr;
  uint64_t bitCount;

public:
  Bitmap(); // for placement-new placeholder only
  Bitmap(uint8_t * ptr, uint64_t count);
  Bitmap(const Bitmap & bm);
  Bitmap & operator=(const Bitmap & bm);
  
  void SetBit(uint64_t idx, bool value);
  void SetMultibit(uint64_t idx, int len, uint64_t value);
  bool GetBit(uint64_t idx) const;
  uint64_t GetMultibit(uint64_t idx, int len) const;
  
  /**
   * Returns the number of bits in the bitmap
   */
  uint64_t GetBitCount() const;
};

}

#endif
