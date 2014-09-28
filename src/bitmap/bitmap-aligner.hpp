#ifndef __ANALLOC2_BITMAP_ALIGNER_HPP__
#define __ANALLOC2_BITMAP_ALIGNER_HPP__

#include "../abstract/aligner.hpp"
#include "bitmap-allocator.hpp"

namespace analloc {

template <typename Unit, typename AddressType, typename SizeType = AddressType>
class BitmapAligner : public Aligner<AddressType, SizeType>,
                      public BitmapAllocator<Unit, AddressType, SizeType> {
public:
  virtual bool Align(AddressType & addressOut, AddressType align,
                     SizeType size) {
    if (!align) return Align(addressOut, 1, size);
    SizeType freeSoFar = 0;
    size_t startIdx = 0;
    // Iterate through each bit. For each unallocated bit, we increment 
    // freeSoFar. If freeSoFar reaches the requested size, we have found a
    // sufficiently large region of free space.
    for (size_t i = 0; i < this->GetBitCount(); ++i) {
      if (!freeSoFar) {
        // Skip to the next aligned region
        if (i % (size_t)align) {
          i += (size_t)align - (i % (size_t)align) - 1;
          continue;
        }
      }
      if (this->GetBit(i)) {
        // This cell is not free, so freeSoFar gets reset.
        freeSoFar = 0;
        continue;
      }
      if (freeSoFar) {
        ++freeSoFar;
      } else {
        // This is the beginning of the new region
        freeSoFar = 1;
        startIdx = i;
      }
      if (freeSoFar == size) {
        // We have found a free region, now we must mark it as allocated.
        for (size_t j = startIdx; j <= i; ++j) {
          this->SetBit(j, true);
        }
        addressOut = (AddressType)startIdx + this->baseAddress;
        return true;
      }
    }
    return false;
  }
};

}

#endif
