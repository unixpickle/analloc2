#ifndef __ANALLOC2_VIRTUAL_BITMAP_ALLOCATOR_HPP__
#define __ANALLOC2_VIRTUAL_BITMAP_ALLOCATOR_HPP__

#include "transformed-bitmap-allocator.hpp"
#include "../wrappers/allocator-virtualizer.hpp"
#include <new>

namespace analloc {

template <typename Unit = unsigned int>
class VirtualBitmapAllocator
    : public AllocatorVirtualizer<
          TransformedBitmapAllocator<Unit, uintptr_t, size_t>
      > {
public:
  typedef AllocatorVirtualizer<
      TransformedBitmapAllocator<Unit, uintptr_t, size_t>
  > super;
  
  /**
   * Create a [VirtualBitmapAllocator] which is embedded in a region of memory.
   * 
   * Returns `nullptr` if there is literally not enough space to generate a
   * [VirtualBitmapAllocator] object with the proper page size and unit
   * alignments.
   */
  static VirtualBitmapAllocator * Place(uintptr_t region, size_t size,
                                        size_t page = sizeof(uintptr_t)) {
    size_t align = ansa::Align(sizeof(Unit), page);
    
    // Compute the number of bytes we have for the bitmap and buffers combined.
    size_t structureSize = ansa::Align(sizeof(VirtualBitmapAllocator), align);
    if (structureSize > size) {
      return nullptr;
    }
    size_t usableSize = ((size - structureSize) / page) * page;
    
    // If Unit were uint8_t, this would be the number of units to use. I did
    // some math to find this. I solved: 8*page*ideal = usable - ideal
    size_t idealBitmapSize = usableSize / (8 * page + 1);
    
    // Align the ideal bitmap size.
    size_t bitmapSize = ansa::Align(idealBitmapSize, align);
    
    // Figure out if we've overflowed.
    if (bitmapSize > usableSize) {
      // Return an empty allocator.
      return new((void *)region) VirtualBitmapAllocator(page,
          region + structureSize, (Unit *)(region + structureSize), 0);
    }
    
    void * ptr = (void *)region;
    Unit * buffer = (Unit *)(region + structureSize);
    uintptr_t offset = region + structureSize + bitmapSize;
    size_t freeSize = ansa::Min(usableSize - bitmapSize,
                                page * bitmapSize * 8);
    assert(!(freeSize % page));
    
    return new(ptr) VirtualBitmapAllocator(page, offset, buffer, freeSize);
  }
  
  VirtualBitmapAllocator(size_t pageSize, uintptr_t _offset,
                         Unit * ptr, size_t size)
      : super(pageSize, pageSize, _offset, ptr, size / pageSize) {}
};

}

#endif
