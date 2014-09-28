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
        addressOut = (AddressType)startIdx + baseAddress;
        return true;
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
