#ifndef __ANALLOC2_OFFSET_ALIGNER_HPP__
#define __ANALLOC2_OFFSET_ALIGNER_HPP__

#include "aligner.hpp"

namespace analloc {

template <typename AddressType, typename SizeType = AddressType>
class OffsetAligner : public virtual Aligner<AddressType, SizeType> {
public:
  virtual bool Align(AddressType & addressOut, AddressType align,
                     SizeType size) {
    return OffsetAlign(addressOut, align, 0, size);
  }
  
  /**
   * Returns an address [addressOut] such that
   * ([addressOut] + [offset]) % [align] == 0.
   */
  virtual bool OffsetAlign(AddressType & addressOut, AddressType align,
                           AddressType offset, SizeType size) = 0;
};

}

#endif
