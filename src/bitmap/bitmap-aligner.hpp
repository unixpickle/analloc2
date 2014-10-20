#ifndef __ANALLOC2_BITMAP_ALIGNER_HPP__
#define __ANALLOC2_BITMAP_ALIGNER_HPP__

#include "../abstract/offset-aligner.hpp"
#include "bitmap-allocator.hpp"

namespace analloc {

/**
 * An aligner that runs in O(n) time.
 */
template <typename Unit, typename AddressType, typename SizeType = AddressType>
class BitmapAligner
    : public BitmapAllocator<Unit, AddressType, SizeType>,
      public virtual OffsetAligner<AddressType, SizeType> {
public:
  BitmapAligner(Unit * ptr, SizeType bc)
      : BitmapAllocator<Unit, AddressType, SizeType>(ptr, bc) {}
  
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
  
protected:
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
};

}

#endif
