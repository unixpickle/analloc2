#ifndef __ANALLOC2_VIRTUAL_BITMAP_ALIGNER_HPP__
#define __ANALLOC2_VIRTUAL_BITMAP_ALIGNER_HPP__

#include "transformed-bitmap-aligner.hpp"
#include "virtual-bitmap-allocator.hpp"
#include "../wrappers/aligner-virtualizer.hpp"

namespace analloc {

template <typename Unit>
class VirtualBitmapAligner
    : public AlignerVirtualizer<
          TransformedBitmapAligner<Unit, uintptr_t, size_t>
      > {
public:
  typedef AlignerVirtualizer<
      TransformedBitmapAligner<Unit, uintptr_t, size_t>
  > super;
  
  /**
   * Create a [VirtualBitmapAligner] which is embedded in a region of memory.
   * 
   * Returns `nullptr` if there is not enough space to generate a
   * [VirtualBitmapAligner] object with the proper page size and unit
   * alignments.
   */
  static VirtualBitmapAligner<Unit> * Place(uintptr_t region, size_t size,
                                              size_t page =
                                                  sizeof(uintptr_t)) {
    // This might be perhaps the longest method call I have ever seen.
    return VirtualBitmapAllocator<Unit>::
           template PlaceObject<VirtualBitmapAligner<Unit> >
           (region, size, page);
  }
  
  VirtualBitmapAligner(size_t pageSize, uintptr_t _offset, Unit * ptr,
                       size_t size)
      : super(pageSize, pageSize, _offset, ptr, size / pageSize) {}
  
  inline size_t GetScale() {
    return this->wrapped.GetScale();
  }

  inline uintptr_t GetOffset() {
    return this->wrapped.GetOffset();
  }
  
  inline size_t GetBitCount() {
    return this->wrapped.GetBitCount();
  }
  
  inline size_t GetTotalSize() {
    return GetBitCount() * GetScale();
  }
};

}

#endif
