#ifndef __ANALLOC2_ALIGNER_VIRTUALIZER_HPP__
#define __ANALLOC2_ALIGNER_VIRTUALIZER_HPP__

#include "allocator-virtualizer.hpp"
#include "../abstract/virtual-offset-aligner.hpp"

namespace analloc {

template <typename T>
class AlignerVirtualizer
    : public AllocatorVirtualizer<T>,
      public virtual VirtualOffsetAligner {
public:
  typedef AllocatorVirtualizer<T> super;
  using typename super::Header;
  
  template <typename... Args>
  AlignerVirtualizer(Args... args) : super(args...) {}
  
  virtual bool Align(uintptr_t & addressOut, uintptr_t align, size_t size) {
    return this->OffsetAlign(addressOut, align, 0, size);
  }
  
  virtual bool OffsetAlign(uintptr_t & addressOut, uintptr_t align,
                           uintptr_t offset, size_t size) {
    uintptr_t buffer;
    if (!this->wrapped.OffsetAlign(buffer, align, offset + this->headerSize,
                                   size + this->headerSize)) {
      return false;
    }
    // Set the size in the header
    Header * header = (Header *)buffer;
    header->size = size;
    // Return the part of the buffer after the header
    addressOut = buffer + this->headerSize;
    return true;
  }
};

}

#endif
