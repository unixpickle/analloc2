#ifndef __ANALLOC2_BITMAP_ALLOCATOR_HPP__
#define __ANALLOC2_BITMAP_ALLOCATOR_HPP__

#include "../abstract/allocator.hpp"
#include "bitmap.hpp"

namespace analloc {

/**
 * An allocator which runs with O(n) allocation time.
 */
template <typename Unit, typename AddressType, typename SizeType = AddressType>
class BitmapAllocator : public Allocator<AddressType, SizeType>,
                        public Bitmap<Unit> {
public:
  BitmapAllocator(AddressType base, Unit * ptr, size_t bc)
      : Bitmap<Unit>(ptr, bc), baseAddress(base) {
    // TODO: zero out the buffer more efficiently
    for (size_t i = 0; i < bc; ++i) {
      this->SetBit(i, 0);
    }
  }
  
  virtual bool Alloc(AddressType & addressOut, SizeType size) {
    SizeType freeSoFar = 0;
    size_t startIdx = 0;
    for (size_t i = 0; i < this->GetBitCount(); ++i) {
      if (this->GetBit(i)) {
        if (freeSoFar) {
          ++freeSoFar;
        } else {
          freeSoFar = 1;
          startIdx = i;
        }
        if (freeSoFar == size) {
          for (size_t j = startIdx; j <= i; ++j) {
            this->SetBit(j, true);
          }
          addressOut = (AddressType)startIdx + baseAddress;
          return true;
        }
      } else {
        freeSoFar = 0;
      }
    }
    return false;
  }
  
  virtual void Dealloc(AddressType address, SizeType size) {
    size_t idx = (size_t)(address - baseAddress);
    assert(idx < this->GetBitCount() && idx + size <= this->GetBitCount());
    for (size_t i = idx; i < idx + size; ++i) {
      this->SetBit(i, false);
    }
  }
  
private:
  AddressType baseAddress;
};

}

#endif
