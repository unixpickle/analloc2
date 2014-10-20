#ifndef __ANALLOC2_TRANSFORMED_BITMAP_ALIGNER_HPP__
#define __ANALLOC2_TRANSFORMED_BITMAP_ALIGNER_HPP__

#include "../abstract/aligner.hpp"
#include "transformed-bitmap-allocator.hpp"
#include <ansa/math>
#include <cassert>

namespace analloc {

template <typename Unit, typename AddressType, typename SizeType = AddressType>
class TransformedBitmapAligner
    : public TransformedBitmapAllocator<Unit, AddressType, SizeType>,
      public virtual Aligner<AddressType, SizeType> {
public:
  typedef TransformedBitmapAllocator<Unit, AddressType, SizeType> super;
  
  /**
   * This is restricted compared to [TransformedBitmapAllocator].
   * 
   * The [_scale] ought to be a power of two, and the [_offset] ought to be
   * aligned by the [_scale].
   */
  template <typename... Args>
  TransformedBitmapAligner(SizeType _scale, AddressType _offset, Unit * ptr,
                           AddressType bc) : super(_scale, _offset, ptr, bc) {
    assert(ansa::IsPowerOf2(_scale));
    assert(ansa::IsAligned<AddressType>(_offset, _scale));
  }
  
  virtual bool Align(AddressType & addressOut, AddressType unscaledAlign,
                     SizeType unscaledSize) {
    AddressType align = ScaleAlign(unscaledAlign);
    SizeType size = this->ScaleSize(unscaledSize);
    if (align <= 1) {
      // In this case, the alignment is less than the minimum unit size anyway.
      return this->Alloc(addressOut, unscaledSize);
    }
    SizeType index = 0;
    while (NextFreeAligned(index, size - 1, align)) {
      if (this->Reserve(index + 1, size - 1, &index)) {
        this->SetBit(index, true);
        addressOut = this->OutputAddress((AddressType)index);
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
  
  bool NextFreeAligned(SizeType & idx, SizeType afterSize,
                       AddressType align) {
    // This is the offset of this allocator in terms of cells in the bitmap,
    // rather than in terms of the scaled output units.
    AddressType cellOffset = this->offset / this->scale;
    
    for (SizeType i = idx; i < this->GetBitCount() - afterSize; ++i) {
      // Skip to the next aligned region
      assert(!ansa::AddWraps<AddressType>(i, cellOffset));
      AddressType misalignment = (cellOffset + i) % align;
      if (misalignment) {
        AddressType add = align - misalignment;
        if ((SizeType)add != add) {
          return false;
        } else if (ansa::AddWraps<SizeType>(i, add)) {
          return false;
        }
        i += add - 1;
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
