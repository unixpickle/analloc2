#ifndef __ANALLOC2_BITMAP_ALLOCATOR_HPP__
#define __ANALLOC2_BITMAP_ALLOCATOR_HPP__

#include "../abstract/allocator.hpp"
#include "bitmap.hpp"

namespace analloc {

/**
 * An allocator which runs with O(n) allocation time.
 */
template <typename Unit, typename AddressType, typename SizeType = AddressType>
class BitmapAllocator : public Bitmap<Unit>,
                        public virtual Allocator<AddressType, SizeType> {
public:
  BitmapAllocator(Unit * ptr, size_t bc) : Bitmap<Unit>(ptr, bc) {
    // Zero the buffer as efficiently as possible without overwriting any bits
    // that this bitmap doesn't own
    size_t fullUnits = bc / (sizeof(Unit) * 8);
    for (size_t i = 0; i < fullUnits; ++i) {
      ptr[i] = 0;
    }
    for (size_t i = fullUnits * sizeof(Unit) * 8; i < bc; ++i) {
      this->SetBit(i, 0);
    }
  }
  
  virtual bool Alloc(AddressType & addressOut, SizeType size) {
    SizeType freeSoFar = 0;
    size_t startIdx = 0;
    // Iterate through each bit. For each unallocated bit, we increment 
    // freeSoFar. If freeSoFar reaches the requested size, we have found a
    // sufficiently large region of free space.
    for (size_t i = 0; i < this->GetBitCount(); ++i) {
      if (this->GetBit(i)) {
        // This cell is not free, so freeSoFar gets reset.
        freeSoFar = 0;
        
        // Skip the entire unit if we are at the beginning of it and it doesn't
        // pass this bitmap's allowed bit range (i.e. the bitmap ends
        // mid-unit).
        if (!(i % this->UnitBitCount) && i + this->UnitBitCount <=
            this->GetBitCount()) {
          if (!~(this->units[i / this->UnitBitCount])) {
            i += this->UnitBitCount - 1;
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
  
  virtual void Dealloc(AddressType address, SizeType size) {
    size_t idx = (size_t)address;
    assert(idx < this->GetBitCount() && idx + size <= this->GetBitCount());
    for (size_t i = idx; i < idx + size; ++i) {
      this->SetBit(i, false);
    }
  }
};

}

#endif
