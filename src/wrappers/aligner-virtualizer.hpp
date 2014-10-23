#ifndef __ANALLOC2_ALIGNER_VIRTUALIZER_HPP__
#define __ANALLOC2_ALIGNER_VIRTUALIZER_HPP__

#include "allocator-virtualizer.hpp"

namespace analloc {

template <typename T>
class AlignerVirtualizer
    : public AllocatorVirtualizer<T>,
      public virtual VirtualOffsetAligner {
public:
  typedef AllocatorVirtualizer<T> super;
  
  template <typename... Args>
  AlignerVirtualizer(Args... args) : super(args...) {}
  
  bool Align(uintptr_t & addressOut, uintptr_t align, size_t size) {
    return this->OffsetAlign(addressOut, align, 0, size);
  }
  
  bool OffsetAlign(uintptr_t & addressOut, uintptr_t align, uintptr_t offset,
                   size_t size) {
    uintptr_t buffer;
    if (!T::OffsetAlign(buffer, align, offset + this->headerSize, size)) {
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
