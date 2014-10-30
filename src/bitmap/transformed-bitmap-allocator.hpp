#ifndef __ANALLOC2_TRANSFORMED_BITMAP_ALLOCATOR_HPP__
#define __ANALLOC2_TRANSFORMED_BITMAP_ALLOCATOR_HPP__

#include "bitmap.hpp"
#include "../wrappers/allocator-transformer.hpp"

namespace analloc {

/**
 * This is essentially a type alias for
 * `AllocatorTransformer<BitmapAllocator<Unit, AddressType, SizeType> >`.
 */
template <typename Unit, typename AddressType, typename SizeType = AddressType>
class TransformedBitmapAllocator
    : public AllocatorTransformer<Bitmap<Unit, AddressType, SizeType> > {
public:
  typedef AllocatorTransformer<Bitmap<Unit, AddressType, SizeType> > super;
  
  TransformedBitmapAllocator(SizeType _scale, AddressType _offset, Unit * ptr,
                             AddressType bc)
      : super(_scale, _offset, ptr, bc) {}
  
  inline SizeType GetBitCount() const {
    return this->wrapped.GetBitCount();
  }
};

}

#endif
