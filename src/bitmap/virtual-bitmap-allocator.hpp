#ifndef __ANALLOC2_VIRTUAL_BITMAP_ALLOCATOR_HPP__
#define __ANALLOC2_VIRTUAL_BITMAP_ALLOCATOR_HPP__

#include "transformed-bitmap-allocator.hpp"
#include "../wrappers/allocator-virtualizer.hpp"

namespace analloc {

template <typename Unit>
class VirtualBitmapAllocator
    : public AllocatorVirtualizer<
          TransformedBitmapAllocator<Unit, uintptr_t, size_t>
      > {
public:
  typedef public AllocatorVirtualizer<
      TransformedBitmapAllocator<Unit, uintptr_t, size_t>
  > super;
  
  VirtualBitmapAllocator(size_t _scale, uintptr_t _offset, Unit * ptr,
                         uintptr_t bitCount)
      : super(_scale, _offset, ptr, bitCount) {}
};

}

#endif
