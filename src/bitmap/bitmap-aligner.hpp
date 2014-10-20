#ifndef __ANALLOC2_BITMAP_ALIGNER_HPP__
#define __ANALLOC2_BITMAP_ALIGNER_HPP__

#include "../abstract/aligner.hpp"
#include "bitmap-allocator.hpp"

#include <iostream> // TODO: delete this

namespace analloc {

/**
 * An aligner that runs in O(n) time.
 */
template <typename Unit, typename AddressType, typename SizeType = AddressType>
class BitmapAligner : public BitmapAllocator<Unit, AddressType, SizeType>,
                      public virtual Aligner<AddressType, SizeType> {
public:
  BitmapAligner(Unit * ptr, SizeType bc)
      : BitmapAllocator<Unit, AddressType, SizeType>(ptr, bc) {}
  
  virtual bool Align(AddressType & addressOut, AddressType align,
                     SizeType size) {
    if (align >= this->GetBitCount()) {
      // The alignment is too large for any return address other than `0`.
      if (size > this->GetBitCount()) {
        return false;
      } else if (this->Reserve(0, size, nullptr)) {
        addressOut = 0;
        return true;
      } else {
        return false;
      }
    } else if (align < 2 || !size) {
      return this->Alloc(addressOut, size);
    } else {
      SizeType index = 0;
      while (NextFreeAligned(index, size - 1, (SizeType)align)) {
        if (this->Reserve(index + 1, size - 1, &index)) {
          this->SetBit(index, true);
          addressOut = (AddressType)index;
          return true;
        }
      }
      return false;
    }
  }
  
protected:
  bool NextFreeAligned(SizeType & idx, SizeType afterSize, SizeType align) {
    assert(afterSize <= this->GetBitCount());
    for (SizeType i = idx; i < this->GetBitCount() - afterSize; ++i) {
      // Skip to the next aligned region
      SizeType misalignment = (SizeType)i % align;
      if (misalignment) {
        SizeType add = align - misalignment - 1;
        if (ansa::AddWraps<SizeType>(i, add)) {
          return false;
        }
        i += add;
        continue;
      } else if (!this->GetBit(i)) {
        idx = i;
        return true;
      } else if (this->IsUnitAllocated(i)) {
        i += this->UnitBitCount - 1;
      }
    }
    return false;
  }
};

}

#endif
