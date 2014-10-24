#ifndef __ANALLOC2_ALIGNER_TRANSFORMER_HPP__
#define __ANALLOC2_ALIGNER_TRANSFORMER_HPP__

#include "allocator-transformer.hpp"
#include "../abstract/offset-aligner.hpp"

namespace analloc {

/**
 * Transforms an [OffsetAligner].
 */
template <typename T>
class AlignerTransformer
    : public AllocatorTransformer<T>,
      public virtual OffsetAligner<typename T::SizeType,
                                   typename T::AddressType> {
public:
  typedef AllocatorTransformer<T> super;
  using typename super::SizeType;
  using typename super::AddressType;
  
  /**
   * Call the superclass's constructor with some added restrictions.
   * 
   * The [_scale] ought to be a power of two, and the [_offset] ought to be
   * aligned by the [_scale].
   */
  template <typename... Args>
  AlignerTransformer(SizeType _scale, AddressType _offset, Args... args)
      : super(_scale, _offset, args...) {
    assert(ansa::IsPowerOf2(_scale));
    assert(ansa::IsAligned<AddressType>(_offset, _scale));
  }
  
  /**
   * Align address space from the wrapped allocator.
   *
   * This works by calling the wrapped allocator's [OffsetAlign] with a
   * compounded, scaled offset.
   */
  virtual bool OffsetAlign(AddressType & addressOut, AddressType align,
                           AddressType anOffset, SizeType size) {
    if (!ansa::IsAligned<AddressType>(anOffset, this->scale)) {
      return false;
    }
    
    AddressType scaledAlign = ScaleAlign(align);
    SizeType scaledSize = this->ScaleSize(size);
    
    // Check if it's a trivial allocation
    if (scaledAlign <= 1) {
      return this->Alloc(addressOut, size);
    }
    
    // The integer wrap-around from `this->offset + anOffset` is acceptable
    // since the result will be "correct" by definition even if wrap-around
    // occurs.
    AddressType scaledOffset = (this->offset + anOffset) / this->scale;
    AddressType addr;
    if (!this->wrapped.OffsetAlign(addr, scaledAlign, scaledOffset,
                                   scaledSize)) {
      return false;
    }
    addressOut = this->OutputAddress(addr);
    return true;
  }
  
protected:
  inline AddressType ScaleAlign(AddressType align) {
    return ansa::RoundUpDiv<AddressType>(align, this->scale);
  }
};

}

#endif
