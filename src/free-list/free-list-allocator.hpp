#ifndef __ANALLOC2_FREE_LIST_ALLOCATOR_HPP__
#define __ANALLOC2_FREE_LIST_ALLOCATOR_HPP__

#include "../abstract/virtual-allocator.hpp"
#include <new>

namespace analloc {

/**
 * An allocator which allocates and frees address space in O(n), where n is
 * the number of disjoint fragments of free space.
 *
 * This may be useful for virtual memory management where there may be a small
 * number of active memory regions at any given time.
 */
template <typename AddressType, typename SizeType = AddressType>
class FreeListAllocator : public Allocator<AddressType, SizeType> {
public:
  /**
   * The function signature of a callback which a [FreeListAllocator] will call
   * when a free region cannot be recorded because memory could not be
   * obtained.
   */
  typedef void (* FailureHandler)(FreeListAllocator *);
  
  /**
   * Create a new [FreeListAllocator] with no free memory regions.
   *
   * The [anAlloc] argument is saved by this allocator to be used throughout
   * its lifetime. This allocator will be used to allocate free regions.
   *
   * The [Dealloc] routine may need to allocate new memory from [anAlloc]. If
   * this allocation fails, [onAllocFail] will be called with this allocator as
   * the argument.
   */
  FreeListAllocator(VirtualAllocator & anAlloc, FailureHandler onAllocFail)
      : allocator(anAlloc), failureHandler(onAllocFail) {}
  
  virtual bool Alloc(AddressType & out, SizeType size) {
    FreeRegion * reg = firstRegion;
    while (reg) {
      if (reg->size < size) {
        reg = reg->next;
      } else if (reg->size == size) {
        // Remove the region from the list
        if (!reg->last) {
          firstRegion = reg->next;
        } else {
          reg->last->next = reg->next;
        }
        if (reg->next) {
          reg->next->last = reg->last;
        }
        out = reg->start;
        allocator.Dealloc((uintptr_t)reg, sizeof(FreeRegion));
        return true;
      } else {
        // Take a chunk out of the region
        out = reg->start;
        reg->start += size;
      }
    }
    return false;
  }
  
  virtual void Dealloc(AddressType address, SizeType size) {
    // Find the regions that surround the freed address
    FreeRegion * after = firstRegion;
    FreeRegion * before = NULL;
    while (after) {
      if (after->start > address) {
        break;
      } else {
        before = after;
        after = after->next;
      }
    }
    if (before && before->start + before->size == address) {
      // The region before the freed address extends to the freed address.
      before->size += size;
      if (after && after->start == before->start + before->size) {
        // The expanded before region extends all the way to the after region.
        before->size += after->size;
        before->next = after->next;
        if (before->next) {
          before->next->last = before;
        }
        allocator.Dealloc((uintptr_t)after, sizeof(FreeRegion));
      }
    } else if (after && address + size == after->start) {
      // The freed region does not touch the region before it, but it does
      // reach the region after it.
      after->start -= size;
      after->size += size;
    } else {
      // The freed region touches neither the region before it nor the one
      // after it.
      uintptr_t ptr;
      if (!allocator.Alloc(ptr, sizeof(FreeRegion))) {
        failureHandler(this);
        return;
      }
      FreeRegion * reg = (FreeRegion *)ptr;
      reg->last = before;
      reg->next = after;
      if (before) {
        before->next = reg;
      } else {
        firstRegion = reg;
      }
      if (after) {
        after->last = reg;
      }
    }
  }
  
protected:
  struct FreeRegion {
    FreeRegion * next, * last;
    AddressType start;
    SizeType size;
  };
  
  FreeRegion * firstRegion = NULL;
  VirtualAllocator & allocator;
  FailureHandler failureHandler;
};

}

#endif
