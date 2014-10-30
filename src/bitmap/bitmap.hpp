#ifndef __ANALLOC2_BITMAP_HPP__
#define __ANALLOC2_BITMAP_HPP__

#include "../abstract/offset-aligner.hpp"
#include "raw-bitmap.hpp"

namespace analloc {

/**
 * An offset aligner which runs in O(n) time for all operations.
 */
template <typename Unit, typename AddressType, typename SizeType = AddressType>
class Bitmap
    : protected RawBitmap<Unit, SizeType>,
      public virtual OffsetAligner<AddressType, SizeType> {
public:
  typedef RawBitmap<Unit, SizeType> super;
  
  /**
   * Create a new [Bitmap] given a region of memory [ptr] and a bit count [bc].
   */
  Bitmap(Unit * ptr, SizeType bc) : super(ptr, bc) {
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
  
  virtual bool OffsetAlign(AddressType & addressOut, AddressType align,
                           AddressType offset, SizeType size) {
    if (align < 2 || !size) {
      return this->Alloc(addressOut, size);
    }
    SizeType index = 0;
    while (NextFreeAligned(index, offset, align, size - 1)) {
      if (this->Reserve(index + 1, size - 1, &index)) {
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
  
  using super::GetBitCount;
  
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
  
  bool NextFreeAligned(SizeType & idx, AddressType offset,
                       AddressType align, SizeType afterSize) {
    if (afterSize >= this->GetBitCount()) {
      return false;
    }
    for (SizeType i = idx; i < this->GetBitCount() - afterSize; ++i) {
      // Skip to the next aligned region
      AddressType misalignment = (offset + i) % align;
      if (misalignment) {
        AddressType add = align - misalignment;
        if ((SizeType)add != add ||
            ansa::AddWraps<SizeType>(i, (SizeType)add)) {
          return false;
        }
        i += add - 1;
      } else if (!this->GetBit(i)) {
        idx = i;
        return true;
      } else if (this->IsUnitAllocated(i)) {
        i += this->UnitBitCount - 1;
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
