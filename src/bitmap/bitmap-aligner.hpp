#ifndef __ANALLOC2_BITMAP_ALIGNER_HPP__
#define __ANALLOC2_BITMAP_ALIGNER_HPP__

#include "../abstract/aligner.hpp"
#include "bitmap-allocator.hpp"

namespace analloc {

/**
 * An aligner that runs in O(n) time.
 */
template <typename Unit, typename AddressType, typename SizeType = AddressType>
class BitmapAligner : public BitmapAllocator<Unit, AddressType, SizeType>,
                      public virtual Aligner<AddressType, SizeType> {
public:
  BitmapAligner(Unit * ptr, size_t bc)
      : BitmapAllocator<Unit, AddressType, SizeType>(ptr, bc) {}
  
  virtual bool Align(AddressType & addressOut, AddressType align,
                     SizeType size) {
    if (align < 2) {
      return this->Alloc(addressOut, size);
    }
    SizeType freeSoFar = 0;
    size_t startIdx = 0;
    // Iterate through each bit. For each unallocated bit, we increment 
    // freeSoFar. If freeSoFar reaches the requested size, we have found a
    // sufficiently large region of free space.
    for (size_t i = 0; i < this->GetBitCount(); ++i) {
      if (!freeSoFar) {
        // Skip to the next aligned region
        AddressType misalignment = (AddressType)i % align;
        if (misalignment) {
          i += (size_t)(align - misalignment - 1);
          continue;
        }
      }
      if (this->GetBit(i)) {
        // This cell is not free, so freeSoFar gets reset.
        freeSoFar = 0;
        
        if (align < this->UnitBitCount) {
          // Skip the entire unit if we are at the beginning of it and it
          // doesn't pass this bitmap's allowed bit range (i.e. the bitmap ends
          // mid-unit).
          if (!(i % this->UnitBitCount) && i + this->UnitBitCount <=
              this->GetBitCount()) {
            if (!~(this->units[i / this->UnitBitCount])) {
              i += this->UnitBitCount - 1;
            }
          }
        }
        
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
        addressOut = (AddressType)startIdx;
        return true;
      }
    }
    return false;
  }
};

}

#endif
