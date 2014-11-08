#ifndef __ANALLOC2_VIRTUAL_PLACED_FREE_LIST_HPP__
#define __ANALLOC2_VIRTUAL_PLACED_FREE_LIST_HPP__

#include "placed-free-list.hpp"
#include "../wrappers/aligner-virtualizer.hpp"

namespace analloc {

template <size_t StackSize = 0x10>
class VirtualPlacedFreeList
    : public AlignerVirtualizer<PlacedFreeList<StackSize> > {
public:
  typedef AlignerVirtualizer<PlacedFreeList<StackSize> > super;
  typedef typename PlacedFreeList<StackSize>::StackType StackType;
  
  static VirtualPlacedFreeList * Place(uintptr_t start, size_t size,
                                       size_t objectAlign = sizeof(void *)) {
    return PlacedFreeList<StackSize>::
    template PlaceInstance<VirtualPlacedFreeList<StackSize>, size_t>
    (start, size, objectAlign, objectAlign);
  }
  
  /**
   * Add an arbitrary region of memory to this allocator.
   *
   * This is necessary because Dealloc() looks at an extra memory header; thus,
   * it cannot easily be used to add arbitrary regions to an allocator.
   *
   * The passed region will be manipulated to meet the allocator's alignment
   * requirements.
   *
   * Returns the number of bytes that the allocator was actually able to
   * utilize after applying the alignment requirements to the given region.
   */
  size_t AddRegion(uintptr_t start, size_t size) {
    size_t align = this->wrapped.GetChunkSize();
    size_t misalignment = start % align;
    if (misalignment) {
      size_t corrected = align - misalignment;
      if (corrected > size) {
        return 0;
      }
      start += corrected;
      size -= corrected;
    }
    misalignment = size % align;
    if (misalignment) {
      if (misalignment >= size) {
        return 0;
      }
      size -= misalignment;
    }
    this->wrapped.Dealloc(start, size);
    return size;
  }
  
protected:
  template <size_t S>
  friend class PlacedFreeList;
  
  VirtualPlacedFreeList(size_t objectAlign, size_t regionSize,
                        StackType * stack, uintptr_t start, size_t size)
      : super(objectAlign, regionSize, stack, start, size) {}
};

}

#endif
