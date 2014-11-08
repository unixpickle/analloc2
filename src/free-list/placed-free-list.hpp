#ifndef __ANALLOC2_PLACED_FREE_LIST_HPP__
#define __ANALLOC2_PLACED_FREE_LIST_HPP__

#include "chunked-free-list.hpp"
#include "../buffered-stack/dependent-buffered-stack.hpp"
#include <new>

namespace analloc {

/**
 * A [ChunkedFreeList] that operates without an external allocator.
 */
template <size_t StackSize = 0x10>
class PlacedFreeList : public ChunkedFreeList<uintptr_t, size_t> {
public:
  static_assert(StackSize >= 3, "Stack must contain at least three objects.");
  
  typedef DependentBufferedStack<StackSize, uintptr_t, size_t> StackType;
  typedef ChunkedFreeList<uintptr_t, size_t> super;
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
    
    super::Dealloc(address, size);
    bool result = stack.ApplyBuffer();
    assert(result);
    (void)result;
  }
  
  inline size_t GetStackCount() {
    return stack.GetCount();
  }
  
protected:
  template <class T>
  friend class AllocatorVirtualizer;
  
  template <size_t S>
  friend class VirtualPlacedFreeList;
  
  StackType & stack;
  
  template <typename T>
  static T * PlaceInstance(uintptr_t start, size_t size, size_t objectAlign) {
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
    StackType * stack = (StackType *)start;
    T * freeList = (T *)(start + stackSize);
    new((void *)start) StackType(1, StackSize - 1, regionSize,
        StackOverflowHandler);
    new(freeList) T(regionSize, stack, start + metadataSize, remainingSize);
    return freeList;
  }
  
  PlacedFreeList(size_t regionSize, StackType * _stack, uintptr_t start,
                 size_t size)
      : super(regionSize, static_cast<Allocator<uintptr_t, size_t> &>(*_stack),
              GetRegionFailureHandler), stack(*_stack) {
    stack.SetSource(this);
    assert(size > 0);
    assert(ansa::IsPowerOf2(regionSize));
    assert(ansa::IsAligned2(size, regionSize));
    assert(ansa::IsAligned2<uintptr_t>(start, regionSize));
    stack.Dealloc(start, regionSize);
    if (size >= regionSize * 2) {
      stack.Dealloc(start + regionSize, regionSize);
      if (size > regionSize * 2) {
        this->Dealloc(start + regionSize * 2, size - regionSize * 2);
      }
    }
  }
  
private:
  static void StackOverflowHandler(typename StackType::super *, uintptr_t,
                                   size_t) {
    assert(false);
  }
  
  static bool GetRegionFailureHandler(FreeList<uintptr_t, size_t> *) {
    assert(false);
  }
};

}

#endif
