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
  BitmapAligner(Unit * ptr, AddressType bc)
      : BitmapAllocator<Unit, AddressType, SizeType>(ptr, bc) {}
  
  virtual bool Align(AddressType & addressOut, AddressType align,
                     SizeType size) {
    if (align < 2 || !size) {
      return this->Alloc(addressOut, size);
    }
    AddressType index = 0;
    while (NextFreeAligned(index, align)) {
      if (this->Reserve(index, size, &index)) {
        addressOut = index;
        return true;
      }
    }
    return false;
  }
  
protected:
  bool NextFreeAligned(AddressType & idx, AddressType align) {
    for (AddressType i = idx; i < this->GetBitCount(); ++i) {
      // Skip to the next aligned region
      AddressType misalignment = (AddressType)i % align;
      if (misalignment) {
        AddressType add = align - misalignment - 1;
        if (i + add + 1 < i) {
          // Integer wrap-around
          return false;
        }
        i += add;
        continue;
      } else if (!this->GetBit(i)) {
        idx = i;
        return true;
      } else {
        // Attempt to skip the entire unit
        if (!(i % this->UnitBitCount) && i + this->UnitBitCount <=
            this->GetBitCount()) {
          if (!~(this->UnitAt(i / this->UnitBitCount))) {
            i += this->UnitBitCount - 1;
          }
        }
      }
    }
    return false;
  }
};

}

#endif
