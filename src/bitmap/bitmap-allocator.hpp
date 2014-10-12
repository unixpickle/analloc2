#ifndef __ANALLOC2_BITMAP_ALLOCATOR_HPP__
#define __ANALLOC2_BITMAP_ALLOCATOR_HPP__

#include "../abstract/allocator.hpp"
#include "bitmap.hpp"

namespace analloc {

/**
 * An allocator which runs with O(n) allocation time.
 */
template <typename Unit, typename AddressType, typename SizeType = AddressType>
class BitmapAllocator : protected Bitmap<Unit, AddressType>,
                        public virtual Allocator<AddressType, SizeType> {
public:
  BitmapAllocator(Unit * ptr, AddressType bc)
      : Bitmap<Unit, AddressType>(ptr, bc) {
    // Zero the buffer as efficiently as possible without overwriting any bits
    // that this bitmap doesn't own
    AddressType fullUnits = bc / this->UnitBitCount;
    for (AddressType i = 0; i < fullUnits; ++i) {
      ptr[(size_t)i] = 0;
    }
    for (AddressType i = fullUnits * this->UnitBitCount; i < bc; ++i) {
      this->SetBit(i, 0);
    }
  }
  
  virtual bool Alloc(AddressType & addressOut, SizeType size) {
    if (!size) {
      return Alloc(addressOut, 1);
    }
    AddressType index = 0;
    while (NextFree(index)) {
      if (Reserve(index, size, &index)) {
        addressOut = index;
        return true;
      }
    }
    return false;
  }
  
  virtual void Dealloc(AddressType address, SizeType size) {
    assert(address < this->GetBitCount() && address + size
           <= this->GetBitCount() && address + size >= address);
    for (AddressType i = address; i < address + size; ++i) {
      this->SetBit(i, false);
    }
  }
  
protected:
  bool NextFree(AddressType & idx) {
    for (AddressType i = idx; i < this->GetBitCount(); ++i) {
      if (this->GetBit(i)) {
        // Skip this entire unit if we can
        if (!(i % this->UnitBitCount) && i + this->UnitBitCount <=
            this->GetBitCount()) {
          if (!~(this->UnitAt(i / this->UnitBitCount))) {
            i += this->UnitBitCount - 1;
          }
        }
      } else {
        idx = i;
        return true;
      }
    }
    return false;
  }
  
  bool Reserve(AddressType idx, SizeType size, AddressType * firstUsed) {
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
};

}

#endif
