#ifndef __ANALLOC2_BITMAP_H__
#define __ANALLOC2_BITMAP_H__

#include <cstdint>
#include <cstddef>
#include <cassert>

namespace ANAlloc {

/**
 * This stores an optimal representation of a bitmap.
 */
class Bitmap {
protected:
  uint8_t * ptr;
  uintptr_t bitCount;

public:
  Bitmap();
  
  Bitmap(const Bitmap & bm);
  
  Bitmap & operator=(const Bitmap & bm);
  
  /**
   * Create a new bitmap at a given memory location.
   */
  Bitmap(uint8_t * ptr, uintptr_t count);
  
  /**
   * Set a bit at a given index.
   */
  void SetBit(uintptr_t idx, bool value);
  
  /**
   * Set a glob of bits.
   */
  void SetMultibit(uintptr_t idx, int len, uintptr_t value);
  
  /**
   * Return the bit at a given index.
   */
  bool GetBit(uintptr_t idx) const;
  
  /**
   * Get a glob of bits.
   */
  uintptr_t GetMultibit(uintptr_t idx, int len) const;
  
  /**
   * Returns the number of bits in the bitmap
   */
  uintptr_t GetBitCount() const;
};

}

#endif
