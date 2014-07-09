#ifndef __ANALLOC2_BITMAP_HPP__
#define __ANALLOC2_BITMAP_HPP__

#include "../int.hpp"

namespace ANAlloc {

/**
 * This stores an optimal representation of a bitmap.
 */
class Bitmap {
protected:
  uint8_t * ptr;
  UInt bitCount;

public:
  Bitmap(); // for placement-new placeholder only
  Bitmap(uint8_t * ptr, UInt count);
  Bitmap(const Bitmap & bm);
  Bitmap & operator=(const Bitmap & bm);
  
  void SetBit(UInt idx, bool value);
  void SetMultibit(UInt idx, int len, UInt value);
  bool GetBit(UInt idx) const;
  UInt GetMultibit(UInt idx, int len) const;
  
  /**
   * Returns the number of bits in the bitmap
   */
  UInt GetBitCount() const;
};

}

#endif
