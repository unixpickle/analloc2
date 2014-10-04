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
  typedef void (* FailureHandler)(FreeListAllocator<AddressType, SizeType> *);
  
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
  
  /**
   * Deallocates all free regions.
   */
  virtual ~FreeListAllocator() {
    while (firstRegion) {
      Remove(firstRegion);
    }
  }
  
  virtual bool Alloc(AddressType & out, SizeType size) {
    FreeRegion * reg = firstRegion;
    while (reg) {
      if (reg->size < size) {
        reg = reg->next;
      } else if (reg->size == size) {
        // Remove the region from the list
        out = reg->start;
        Remove(reg);
        return true;
      } else {
        // Take a chunk out of the region
        out = reg->start;
        reg->start += size;
        reg->size -= size;
        return true;
      }
    }
    return false;
  }
  
  virtual void Dealloc(AddressType address, SizeType size) {
    // Find the regions that surround the freed address
    FreeRegion * after = firstRegion;
    FreeRegion * before = nullptr;
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
        Remove(after);
      }
    } else if (after && address + size == after->start) {
      // The freed region does not touch the region before it, but it does
      // reach the region after it.
      after->start -= size;
      after->size += size;
    } else {
      // The freed region touches neither the region before it nor the one
      // after it.
      InsertAfter(before, address, size);
    }
  }
  
protected:
  struct FreeRegion {
    FreeRegion * next, * last;
    AddressType start;
    SizeType size;
  };
  
  FreeRegion * firstRegion = nullptr;
  VirtualAllocator & allocator;
  FailureHandler failureHandler;
  
  void InsertAfter(FreeRegion * before, AddressType addr, SizeType size) {
    uintptr_t ptr;
    if (!allocator.Alloc(ptr, sizeof(FreeRegion))) {
      failureHandler(this);
      return;
    }
    FreeRegion * insert = (FreeRegion *)ptr;
    insert->start = addr;
    insert->size = size;
    if (before) {
      if (before->next) {
        before->next->last = insert;
      }
      insert->next = before->next;
      insert->last = before;
      before->next = insert;
    } else {
      if (firstRegion) {
        firstRegion->last = insert;
      }
      insert->next = firstRegion;
      insert->last = nullptr;
      firstRegion = insert;
    }
  }
  
  void Remove(FreeRegion * region) {
    if (region->last) {
      region->last->next = region->next;
    } else {
      firstRegion = region->next;
    }
    if (region->next) {
      region->next->last = region->last;
    }
    allocator.Dealloc((uintptr_t)region, sizeof(FreeRegion));
  }
};

}

#endif
