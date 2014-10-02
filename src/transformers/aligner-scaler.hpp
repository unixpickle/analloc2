#ifndef __ANALLOC2_ALIGNER_SCALER_HPP__
#define __ANALLOC2_ALIGNER_SCALER_HPP__

#include "allocator-scaler.hpp"

namespace analloc {

/**
 * Turn an aligner of class [T] into another aligner which scales the addresses
 * it returns.
 *
 * It should be noted that, for this to work properly, the scale which is used
 * ought to be a power of 2. Additionally, the start address of this
 * transformer needs to be aligned by x in order to align memory aligned by x.
 */
template <class T, typename AddressType, typename SizeType = AddressType>
class AlignerScaler : public AllocatorScaler<T, AddressType, SizeType> {
public:
  template <typename... Args>
  AlignerScaler(AddressType start, AddressType scale, Args... args)
      : AllocatorScaler<T, AddressType, SizeType>(start, scale, args...) {}
  
  virtual bool Align(AddressType & output, AddressType align, SizeType size) {
    if (this->scalerStart % align) {
      return false;
    }
    AddressType alignment = align / this->scalerMultiple;
    if (!T::Align(output, align, this->ScaleSize(size))) {
      return false;
    }
    output = ScaleAddress(output);
  }
};

}

#endif
