#ifndef __ANALLOC2_TRANSFORMED_BITMAP_ALIGNER_HPP__
#define __ANALLOC2_TRANSFORMED_BITMAP_ALIGNER_HPP__

#include "../abstract/aligner.hpp"
#include "transformed-bitmap-allocator.hpp"
#include <ansa/math>
#include <cassert>

namespace analloc {

/**
 * This class is intended to be used as a mixin for any
 * [TransformedBitmapAllocator] subclass. It adds aligner functionality to a
 * transformed bitmap allocator.
 */
template <typename Unit, typename AddressType, typename SizeType = AddressType>
class TransformedBitmapAligner
    : public virtual TransformedBitmapAllocator<Unit, AddressType, SizeType>,
      public virtual Aligner<AddressType, SizeType> {
public:
  typedef TransformedBitmapAllocator<Unit, AddressType, SizeType> super;
  
  /**
   * This is restricted compared to [TransformedBitmapAllocator].
   * 
   * The scale ought to be a power of two, and the offset ought to be aligned
   * by the scale.
   */
  template <typename... Args>
  TransformedBitmapAligner(Args... args) : super(args...) {
    assert(ansa::IsPowerOf2(this->scale));
    assert(ansa::IsAligned(this->offset, this->scale));
  }
  
  virtual bool Align(AddressType & addressOut, AddressType unscaledAlign,
                     SizeType unscaledSize) {
    AddressType align = ScaleAlign(unscaledAlign);
    SizeType size = this->ScaleSize(unscaledSize);
    if (align <= 1) {
      // In this case, the alignment is less than the minimum unit size anyway.
      return this->Alloc(addressOut, unscaledSize);
    }
    AddressType index = 0;
    while (NextFreeAligned(index, size - 1, align)) {
      if (this->Reserve(index + 1, size - 1, &index)) {
        this->SetBit(index, true);
        addressOut = this->OutputAddress(index);
        return true;
      }
    }
    return false;
  }
  
protected:
  inline AddressType ScaleAlign(AddressType align) {
    AddressType scaledAlign = align / this->scale;
    if (align % this->scale) ++scaledAlign;
    return scaledAlign;
  }
  
  bool NextFreeAligned(AddressType & idx, SizeType afterSize,
                       AddressType align) {
    // This is the offset of this allocator in terms of cells in the bitmap,
    // rather than in terms of the scaled output units.
    AddressType cellOffset = this->offset / this->scale;
    
    for (AddressType i = idx; i < this->GetBitCount() - afterSize; ++i) {
      // Skip to the next aligned region
      assert(!ansa::AddWraps<AddressType>(i, cellOffset));
      AddressType misalignment = (AddressType)(i + cellOffset) % align;
      if (misalignment) {
        AddressType add = align - misalignment - 1;
        if (ansa::AddWraps<AddressType>(i, add)) {
          return false;
        }
        i += add;
        continue;
      } else if (!this->GetBit(i)) {
        idx = i;
        return true;
      } else {
        // Attempt to skip the entire unit
        if (!(i % this->UnitBitCount)
            && i + this->UnitBitCount <= this->GetBitCount()
            && !ansa::AddWraps<AddressType>(i, this->UnitBitCount)) {
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
