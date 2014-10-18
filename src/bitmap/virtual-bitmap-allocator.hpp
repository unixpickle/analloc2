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
  template <class T = VirtualBitmapAllocator<Unit> >
  static T * Place(uintptr_t region, size_t size,
                   size_t page = sizeof(uintptr_t)) {
    size_t align = ansa::Align(sizeof(Unit), page);
    
    // Compute the number of bytes we have for the bitmap and buffers combined.
    size_t structureSize = ansa::Align(sizeof(T), align);
    if (structureSize > size) {
      return nullptr;
    }
    size_t usableSize = ((size - structureSize) / page) * page;
    
    // Compute the size of the actual bitmap data
    size_t freeSize;
    size_t bitmapSize = BestBitmapSize(usableSize, page, freeSize);
    assert(!(bitmapSize % align));
    assert(!(freeSize % page));
    
    void * ptr = (void *)region;
    Unit * buffer = (Unit *)(region + structureSize);
    uintptr_t offset = region + structureSize + bitmapSize;
    
    return new(ptr) T(page, offset, buffer, freeSize);
  }
  
  VirtualBitmapAllocator(size_t pageSize, uintptr_t _offset,
                         Unit * ptr, size_t size)
      : super(pageSize, pageSize, _offset, ptr, size / pageSize) {}
  
protected:
  static size_t BestBitmapSize(size_t usableSize, size_t pageSize,
                               size_t & freeSize) {
    size_t align = ansa::Align(sizeof(Unit), pageSize);
    if (align >= usableSize) {
      freeSize = 0;
      return 0;
    }
    
    // Use the byte count that results from a basic division operation
    size_t byteCount = ansa::Align(usableSize / (pageSize * 8 + 1), align);
    size_t result = byteCount;
    freeSize = FreeSizeForBitmap(usableSize, pageSize, byteCount);
    // Try rounding down
    // TODO: rounding down may actually never be necessary
    if (byteCount > 0) {
      size_t f = FreeSizeForBitmap(usableSize, pageSize, byteCount - align);
      if (f > freeSize) {
        freeSize = f;
        result = byteCount - align;
      }
    }
    // Try rounding up
    if (!ansa::AddWraps<size_t>(byteCount, align) &&
        byteCount + align < usableSize) {
      size_t f = FreeSizeForBitmap(usableSize, pageSize, byteCount + align);
      if (f > freeSize) {
        freeSize = f;
        result = byteCount + align;
      }
    }
    return result;
  }
  
  static size_t FreeSizeForBitmap(size_t usableSize, size_t pageSize,
                                  size_t bitmapSize) {
    assert(ansa::IsAligned(bitmapSize, ansa::Align(sizeof(Unit), pageSize)));
    size_t unitCount = bitmapSize / sizeof(Unit);
    size_t representable = pageSize * unitCount * sizeof(Unit) * 8;
    return ansa::Min(usableSize - bitmapSize, representable);
  }
};

}

#endif
