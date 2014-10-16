#ifndef __ANALLOC2_BITMAP_ALLOCATOR_HPP__
#define __ANALLOC2_BITMAP_ALLOCATOR_HPP__

#include "../abstract/allocator.hpp"
#include "bitmap.hpp"

namespace analloc {

/**
 * An allocator which runs with O(n) allocation time.
 */
template <typename Unit, typename AddressType, typename SizeType = AddressType>
class BitmapAllocator : protected Bitmap<Unit, SizeType>,
                        public virtual Allocator<AddressType, SizeType> {
public:
  BitmapAllocator(Unit * ptr, SizeType bc)
      : Bitmap<Unit, SizeType>(ptr, bc) {
    // Zero the buffer as efficiently as possible without overwriting any bits
    // that this bitmap doesn't own
    SizeType fullUnits = bc / this->UnitBitCount;
    for (SizeType i = 0; i < fullUnits; ++i) {
      ptr[(size_t)i] = 0;
    }
    for (SizeType i = fullUnits * this->UnitBitCount; i < bc; ++i) {
      this->SetBit(i, 0);
    }
  }
  
  virtual bool Alloc(AddressType & addressOut, SizeType size) {
    // Special cases
    if (size > this->GetBitCount()) {
      return false;
    } else if (!size) {
      addressOut = 0;
      return true;
    }
    // Keep attempting to find free regions
    SizeType index = 0;
    while (NextFree(index, size - 1)) {
      if (Reserve(index + 1, size - 1, &index)) {
        this->SetBit(index, true);
        addressOut = (AddressType)index;
        return true;
      }
    }
    return false;
  }
  
  virtual void Dealloc(AddressType address, SizeType size) {
    assert((SizeType)address == address);
    assert(!ansa::AddWraps<SizeType>((SizeType)address, size));
    assert((SizeType)address + size <= this->GetBitCount());
    for (SizeType i = (SizeType)address; i < (SizeType)address + size; ++i) {
      this->SetBit(i, false);
    }
  }
  
protected:
  bool NextFree(SizeType & idx, SizeType afterSize) {
    assert(afterSize <= this->GetBitCount());
    for (SizeType i = idx; i < this->GetBitCount() - afterSize; ++i) {
      if (this->GetBit(i)) {
        // Skip this entire unit if we can
        if (IsUnitAllocated(i)) {
          // We subtract 1 from UnitBitCount for the ++i in the for-loop.
          i += this->UnitBitCount - 1;
        }
      } else {
        idx = i;
        return true;
      }
    }
    return false;
  }
  
  bool Reserve(SizeType idx, SizeType size, SizeType * firstUsed) {
    assert(!ansa::AddWraps<SizeType>(idx, size));
    assert(idx + size <= this->GetBitCount());
    // Make sure that the next [size] cells are free.
    for (SizeType i = 0; i < size; ++i) {
      if (this->GetBit(idx + i)) {
        if (firstUsed) {
          (*firstUsed) = idx + i;
        }
        return false;
      }
    }
    // Reserve all the cells
    for (SizeType i = 0; i < size; ++i) {
      this->SetBit(idx + i, true);
    }
    return true;
  }
  
  /**
   * Returns `true` only if the entire unit starting with [idx] is allocated.
   */
  bool IsUnitAllocated(SizeType i) {
    // I love short-circuit evaluation; don't you?
    return !(i % this->UnitBitCount) &&
           i + this->UnitBitCount <= this->GetBitCount() &&
           !ansa::AddWraps<SizeType>(i, this->UnitBitCount) &&
           !~(this->UnitAt(i / this->UnitBitCount));
  }
};

}

#endif
