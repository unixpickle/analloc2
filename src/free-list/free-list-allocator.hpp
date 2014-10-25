#ifndef __ANALLOC2_FREE_LIST_ALLOCATOR_HPP__
#define __ANALLOC2_FREE_LIST_ALLOCATOR_HPP__

#include "../abstract/virtual-allocator.hpp"
#include <ansa/math>
#include <new>

namespace analloc {

/**
 * An allocator which allocates and frees address space in O(n), where n is
 * the number of disjoint fragments of free space.
 *
 * This may be useful for virtual memory management where there is a small
 * number of active memory regions at any given time.
 */
template <typename AddressType, typename SizeType = AddressType>
class FreeListAllocator : public virtual Allocator<AddressType, SizeType> {
public:
  /**
   * The function signature of a callback which a [FreeListAllocator] will call
   * when a free region cannot be recorded because memory could not be
   * obtained.
   *
   * If this function returns true, the caller will re-attempt the allocation.
   */
  typedef bool (* FailureHandler)(FreeListAllocator<AddressType, SizeType> *);
  
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
      Remove(nullptr, firstRegion);
    }
  }
  
  virtual bool Alloc(AddressType & out, SizeType size) {
    FreeRegion * last = nullptr;
    FreeRegion * reg = firstRegion;
    while (reg) {
      if (reg->size < size) {
        last = reg;
        reg = reg->next;
      } else if (reg->size == size) {
        // Remove the region from the list
        out = reg->start;
        Remove(last, reg);
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
      assert(!ansa::AddWraps<SizeType>(before->size, size));
      before->size += size;
      if (after && after->start == before->start + before->size) {
        // The expanded before region extends all the way to the after region.
        assert(!ansa::AddWraps<SizeType>(before->size, after->size));
        before->size += after->size;
        Remove(before, after);
      }
    } else if (after && address + size == after->start) {
      assert(!ansa::AddWraps<SizeType>(after->size, size));
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
  
  struct FreeRegion {
    FreeRegion * next;
    AddressType start;
    SizeType size;
  };

protected:  
  FreeRegion * firstRegion = nullptr;
  VirtualAllocator & allocator;
  FailureHandler failureHandler;
  
  void InsertAfter(FreeRegion * before, AddressType addr, SizeType size) {
    uintptr_t ptr;
    while (!allocator.Alloc(ptr, sizeof(FreeRegion))) {
      if (!failureHandler(this)) {
        return;
      }
    }
    FreeRegion * insert = (FreeRegion *)ptr;
    insert->start = addr;
    insert->size = size;
    if (before) {
      insert->next = before->next;
      before->next = insert;
    } else {
      insert->next = firstRegion;
      firstRegion = insert;
    }
  }
  
  void Remove(FreeRegion * last, FreeRegion * region) {
    if (last) {
      last->next = region->next;
    } else {
      firstRegion = region->next;
    }
    allocator.Dealloc((uintptr_t)region, sizeof(FreeRegion));
  }
};

}

#endif
