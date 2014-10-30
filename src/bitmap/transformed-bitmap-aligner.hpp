#ifndef __ANALLOC2_TRANSFORMED_BITMAP_ALIGNER_HPP__
#define __ANALLOC2_TRANSFORMED_BITMAP_ALIGNER_HPP__

#include "bitmap.hpp"
#include "../wrappers/aligner-transformer.hpp"

namespace analloc {

template <typename Unit, typename AddressType, typename SizeType = AddressType>
class TransformedBitmapAligner
    : public AlignerTransformer<Bitmap<Unit, AddressType, SizeType> > {
public:
  typedef AlignerTransformer<Bitmap<Unit, AddressType, SizeType> > super;
  
  /**
   * The [_scale] ought to be a power of two, and the [_offset] ought to be
   * aligned by the [_scale].
   */
  template <typename... Args>
  TransformedBitmapAligner(SizeType _scale, AddressType _offset, Unit * ptr,
                           SizeType bc) : super(_scale, _offset, ptr, bc) {
    assert(ansa::IsPowerOf2(_scale));
    assert(ansa::IsAligned<AddressType>(_offset, _scale));
  }
  
  inline SizeType GetBitCount() const {
    return this->wrapped.GetBitCount();
  }
};

}

#endif
