#ifndef __ANALLOC2_PLACED_FREE_LIST_HPP__
#define __ANALLOC2_PLACED_FREE_LIST_HPP__

#include "chunked-free-list.hpp"
#include "../buffered-stack/virtual-buffered-stack.hpp"
#include <new>

namespace analloc {

/**
 * A [ChunkedFreeList] that operates without an external allocator.
 */
template <size_t StackSize = 0x10>
class PlacedFreeList : public ChunkedFreeList<uintptr_t, size_t> {
public:
  static_assert(StackSize >= 4, "Stack must contain at least four objects.");
  
  typedef VirtualBufferedStack<StackSize> StackType;
  typedef VirtualFreeList super;
  using super::FreeRegion;
  
  static PlacedFreeList * Place(uintptr_t start, size_t size,
                                size_t objectAlign = sizeof(void *)) {
    return PlaceInstance<PlacedFreeList>(start, size, objectAlign);
  }
  
  virtual bool Alloc(uintptr_t & addressOut, size_t size) {
    if (!super::Alloc(addressOut, size)) {
      return false;
    }
    bool result = stack.ApplyBuffer();
    assert(result);
    (void)result;
    return true;
  }
  
  virtual bool OffsetAlign(uintptr_t & addressOut, uintptr_t align,
                           uintptr_t offset, size_t size) {
    if (!super::OffsetAlign(addressOut, align, offset, size)) {
      return false;
    }
    bool result = stack.ApplyBuffer();
    assert(result);
    (void)result;
    return true;
  }
  
  virtual void Dealloc(uintptr_t address, size_t size) {
    // Zero-sized deallocations are no good; we need at least enough room in
    // the deallocation to store a region structure.
    if (!size) {
      return;
    }
    
    if (!super::Dealloc(address, size)) {
      return false;
    }
    
    bool result = stack.ApplyBuffer();
    assert(result);
    (void)result;
  }
  
protected:
  StackType & stack;
  
  template <typename T, typename... Args>
  T * PlaceInstance(uintptr_t start, size_t size, size_t objectAlign,
                    Args... constructorArgs) {
    assert(ansa::IsPowerOf2(objectAlign));
    assert(ansa::IsAligned2(start, objectAlign));
    
    // The region size must be a power of two and must be aligned by
    // [objectAlign].
    size_t regionSize = ansa::Align2((size_t)1 << 
        ansa::Log2Ceil(sizeof(super::FreeRegion)), objectAlign);
    
    size_t instanceSize = ansa::Align2(sizeof(T), objectAlign);
    size_t stackSize = ansa::Align2(sizeof(StackType), objectAlign);
    size_t metadataSize = instanceSize + stackSize;
    
    // Since all memory in the allocator must be aligned by [regionSize], we
    // might have to add some padding.
    size_t misalignment = (size_t)((start + metadataSize) % regionSize);
    if (misalignment) {
      metadataSize += regionSize - misalignment;
    }
    
    // Make sure there is enough room to store one region
    if (metadataSize + regionSize > size) {
      return nullptr;
    }
    
    // Align the remaining size by [regionSize].
    size_t remainingSize = size - metadataSize;
    remainingSize &= ~(regionSize - 1);
    
    // Welcome to placement-new hell. Where C++ goes to die.
    T * freeList = (T *)(start + stackSize);
    StackType * stack = (StackType *)start;
    stack = new(stack) StackType(*freeList, 1, StackSize - 2, regionSize,
        StackOverflowHandler);
    freeList = new(freeList) T(constructorArgs..., regionSize, stack,
        start + metadataSize, remainingSize);
    return freeList;
  }
  
  PlacedFreeList(size_t regionSize, StackType * stack, uintptr_t start,
                 size_t size)
      : super(regionSize, stack, GetRegionFailureHandler), stack(*stack) {
    assert(size > 0);
    assert(ansa::IsPowerOf2(regionSize));
    assert(ansa::IsAligned2(size, regionSize));
    assert(ansa::IsAligned2<uintptr_t>(start, regionSize));
    if (size >= regionSize * 2) {
      stack.Free(start);
      stack.Free(start + regionSize);
      if (size > regionSize * 2) {
        this->Dealloc(start + regionSize * 2, size - regionSize * 2);
      }
    } else {
      stack.Free(start);
    }
  }
  
private:
  static void StackOverflowHandler(StackType *, uintptr_t, size_t) {
    assert(false);
  }
  
  static void GetRegionFailureHandler(FreeList<uintptr_t, size_t> *) {
    assert(false);
  }
};

}

#endif
