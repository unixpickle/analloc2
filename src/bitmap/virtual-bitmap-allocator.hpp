#ifndef __ANALLOC2_VIRTUAL_BITMAP_ALLOCATOR_HPP__
#define __ANALLOC2_VIRTUAL_BITMAP_ALLOCATOR_HPP__

#include "transformed-bitmap-allocator.hpp"
#include "../wrappers/allocator-virtualizer.hpp"
#include <new>

namespace analloc {

template <typename Unit>
class VirtualBitmapAligner;

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
   * Returns `nullptr` if there is not enough space to generate a
   * [VirtualBitmapAllocator] object with the proper page size and unit
   * alignments.
   */
  static VirtualBitmapAllocator<Unit> * Place(uintptr_t region, size_t size,
                                              size_t page =
                                                  sizeof(uintptr_t)) {
    return PlaceObject(region, size, page);
  }
  
  VirtualBitmapAllocator(size_t pageSize, uintptr_t _offset,
                         Unit * ptr, size_t size)
      : super(pageSize, pageSize, _offset, ptr, size / pageSize) {}
  
  inline size_t GetScale() {
    return this->wrapped.GetScale();
  }
  
  inline uintptr_t GetOffset() {
    return this->wrapped.GetOffset();
  }
  
protected:
  friend class VirtualBitmapAligner<Unit>;
  
  /**
   * Computes the optimal bitmap size to cover a region of memory.
   *
   * Returns the size of the bitmap in bytes. The bitmap size will be aligned
   * both by sizeof(Unit) and by [pageSize].
   *
   * Sets [freeSize] to the number of bytes which can be covered by the
   * allocator. This may not be usableSize - bitmapSize, in the case where the
   * optimal bitmap doesn't have enough bits to cover the entire buffer.
   */
  static size_t BestBitmapSize(size_t usableSize, size_t pageSize,
                               size_t & freeSize) {
    size_t align = ansa::Align(sizeof(Unit), pageSize);
    if (align >= usableSize) {
      freeSize = 0;
      return 0;
    }
    
    // Use basic division to find a lower bound for the bitmap size.
    size_t byteCount = ansa::Align(usableSize / (pageSize * 8 + 1), align);
    size_t result = byteCount;
    freeSize = FreeSizeForBitmap(usableSize, pageSize, byteCount);
    
    // Attempt to round up and see if we can represent more space.
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
  
  /**
   * Compute the maximum amount of data that can be covered by a bitmap in a
   * given region.
   */
  static size_t FreeSizeForBitmap(size_t usableSize, size_t pageSize,
                                  size_t bitmapSize) {
    assert(ansa::IsAligned(bitmapSize, ansa::Align(sizeof(Unit), pageSize)));
    size_t unitCount = bitmapSize / sizeof(Unit);
    size_t representable = pageSize * unitCount * sizeof(Unit) * 8;
    return ansa::Min(usableSize - bitmapSize, representable);
  }
  
  /**
   * Internal implementation of [Place].
   *
   * The template argument allows subclasses to place themselves in regions of
   * memory, even if they require more storage than [VirtualBitmapAllocator].
   */
  template <class T = VirtualBitmapAllocator<Unit> >
  static T * PlaceObject(uintptr_t region, size_t size, size_t page) {
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
};

}

#endif
