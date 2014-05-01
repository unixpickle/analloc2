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
private:
  uint8_t * ptr;
  uintptr_t bitCount;

public:
  /**
   * Create a new bitmap at a given memory location.
   */
  Bitmap(uint8_t * ptr, uintptr_t count);
  
  /**
   * Set a bit at a given index.
   */
  void SetBit(uintptr_t idx, bool value);
  
  /**
   * Return the bit at a given index.
   */
  bool GetBit(uintptr_t idx) const;
};

}

#endif
