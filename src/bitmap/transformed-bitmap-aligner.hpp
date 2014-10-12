#ifndef __ANALLOC2_TRANSFORMED_BITMAP_ALIGNER_HPP__
#define __ANALLOC2_TRANSFORMED_BITMAP_ALIGNER_HPP__

#include "transformed-bitmap-allocator.hpp"
#include <ansa/math>
#include <cassert>

namespace analloc {

template <typename Unit, typename AddressType, typename SizeType = AddressType>
class TransformedBitmapAligner
    : public TransformedBitmapAllocator<Unit, AddressType, SizeType> {
public:
  typedef TransformedBitmapAllocator<Unit, AddressType, SizeType> super;
  
  /**
   * This is restricted compared to its superclass. The scale ought to be a
   * power of two, and the offset ought to be aligned by the scale.
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
    // Search the bitmap for a large enough free region
    SizeType freeSoFar = 0;
    size_t startIdx = 0;
    for (size_t i = 0; i < this->GetBitCount(); ++i) {
      if (!freeSoFar) {
        // Skip to the next aligned region
        AddressType offsetAddr = this->OutputAddress((AddressType)i);
        AddressType misalignment = offsetAddr % align;
        if (misalignment) {
          AddressType add = align - misalignment - 1;
          if ((size_t)(i + add) < i) {
            // The case where AddressType is larger than size_t and the
            // alignment causes the addresses to be pushed past the size_t
            // range.
            return false;
          }
          i += (size_t)add;
          continue;
        }
      }
      if (this->GetBit(i)) {
        // This cell is not free, so freeSoFar gets reset.
        freeSoFar = 0;
        
        if (align < this->UnitBitCount) {
          // Skip the entire unit if we are at the beginning of it and it
          // doesn't pass this bitmap's allowed bit range (i.e. the bitmap ends
          // mid-unit).
          if (!(i % this->UnitBitCount) && i + this->UnitBitCount <=
              this->GetBitCount()) {
            if (!~(this->units[i / this->UnitBitCount])) {
              i += this->UnitBitCount - 1;
            }
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
        addressOut = OutputAddress((AddressType)startIdx);
        return true;
      }
    }
    return false;
  }
  
protected:
  inline AddressType ScaleAlign(AddressType align) {
    return this->align % this->scale ? this->align / this->scale
        : (this->align / this->scale) + 1;
  }
};

}

#endif
