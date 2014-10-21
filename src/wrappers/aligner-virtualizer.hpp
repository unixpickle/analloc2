#ifndef __ANALLOC2_ALIGNER_VIRTUALIZER_HPP__
#define __ANALLOC2_ALIGNER_VIRTUALIZER_HPP__

#include "allocator-virtualizer.hpp"

namespace analloc {

template <typename T>
class AlignerVirtualizer
    : public AllocatorVirtualizer<T>,
      public virtual VirtualOffsetAligner {
public:
  bool Align(uintptr_t & addressOut, uintptr_t align, size_t size) {
    // TODO: this
  }
  
  bool OffsetAlign(uintptr_t & addressOut, uintptr_t align, size_t size) {
    // TODO: this
  }
};

}

#endif
