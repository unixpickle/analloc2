#ifndef __ANALLOC2_TRANSFORMED_BITMAP_ALLOCATOR_HPP__
#define __ANALLOC2_TRANSFORMED_BITMAP_ALLOCATOR_HPP__

#include "bitmap-allocator.hpp"
#include "../wrappers/allocator-transformer.hpp"

namespace analloc {

/**
 * This is essentially a type alias for
 * `AllocatorTransformer<BitmapAllocator<Unit, AddressType, SizeType> >`.
 */
template <typename Unit, typename AddressType, typename SizeType = AddressType>
class TransformedBitmapAllocator
    : public virtual AllocatorTransformer<BitmapAllocator<Unit, AddressType,
                                                          SizeType> > {
public:
  typedef AllocatorTransformer<BitmapAllocator<Unit, AddressType, SizeType> >
      super;
  
  template <typename... Args>
  TransformedBitmapAllocator(Args... args) : super(args...) {}
};

}

#endif
