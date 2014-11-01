#ifndef __ANALLOC2_FREE_LIST_HPP__
#define __ANALLOC2_FREE_LIST_HPP__

#include "../abstract/offset-aligner.hpp"
#include <ansa/math>
#include <cstdint>
#include <cstddef>

namespace analloc {

/**
 * An offset aligner which allocates and frees address space in O(n), where n
 * is the number of disjoint fragments of free space.
 *
 * This may be useful for virtual memory management where there is a small
 * number of active memory regions at any given time.
 *
 * Note: if you specify an AddressType which is bigger than SizeType, you
 * cannot have contiguous regions of free space which are larger than the
 * maximum value of SizeType.
 */
template <typename AddressType, typename SizeType = AddressType>
class FreeList : public virtual OffsetAligner<AddressType, SizeType> {
public:
  /**
   * The function signature of a callback which a [FreeList] will call when a
   * free region cannot be recorded because memory could not be obtained.
   *
   * If this function returns true, the caller will re-attempt the allocation.
   */
  typedef bool (* FailureHandler)(FreeList<AddressType, SizeType> *);
  
  /**
   * Create a new [FreeList] with no free memory regions.
   *
   * The [anAlloc] argument is saved by this allocator to be used throughout
   * its lifetime. This allocator will be used to allocate free regions.
   *
   * The [Dealloc] routine may need to allocate new memory from [anAlloc]. If
   * this allocation fails, [onAllocFail] will be called with this allocator as
   * the argument.
   */
  FreeList(Allocator<uintptr_t, size_t> & anAlloc, FailureHandler onAllocFail)
      : allocator(anAlloc), failureHandler(onAllocFail) {}
  
  /**
   * Deallocate all free regions.
   */
  virtual ~FreeList() {
    while (firstRegion) {
      Remove(nullptr, firstRegion);
    }
  }
  
  /**
   * Allocate memory from a chunk.
   *
   * This finds the first chunk which is big enough to accommodate the
   * requested allocation. It slices off the beginning of the found chunk and
   * returns it. The chunk will be completely removed if it is exactly [size]
   * units large.
   *
   * If and only if no chunk is found which is big enough to fit [size], this
   * method will return `false`.
   */
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

  /**
   * Align address space.
   *
   * If the first region matching the alignment criteria is nested within a
   * chunk, that chunk will be split into two separate chunks.
   */
  virtual bool OffsetAlign(AddressType & out, AddressType align,
                           AddressType alignOffset, SizeType size) {
    FreeRegion * last = nullptr;
    FreeRegion * reg = this->firstRegion;
    while (reg) {
      // Compute the offset in this region to align it properly.
      SizeType offset = 0;
      AddressType misalignment = (AddressType)(reg->start + alignOffset) %
                                 align;
      if (misalignment) {
        AddressType compensation = align - misalignment;
        offset = (SizeType)compensation;
        if (offset != compensation || offset > reg->size) {
          // The aligned address is out of bounds.
          last = reg;
          reg = reg->next;
          continue;
        }
      }
      // Check for the five cases:
      // - there isn't enough room in this region
      // - there is exactly enough room in this region and offset = 0
      // - there is more than enough room in this region and offset = 0
      // - there is just enough room in this region with offset != 0
      // - there is more than enough room in this region with offset != 0
      if (reg->size - offset < size) {
        reg = reg->next;
      } else if (offset == 0 && size == reg->size) {
        // Remove the region from the list
        out = reg->start;
        this->Remove(last, reg);
        return true;
      } else if (offset == 0 && size < reg->size) {
        // Take the first chunk out of the region
        out = reg->start;
        reg->size -= size;
        reg->start += size;
        return true;
      } else if (offset != 0 && size + offset == reg->size) {
        // Take the last chunk out of the region
        out = reg->start + offset;
        reg->size = offset;
        return true;
      } else {
        // Carve out the middle of the region
        out = reg->start + offset;
        this->InsertAfter(reg, reg->start + offset + size,
                          reg->size - (offset + size));
        reg->size = offset;
        return true;
      }
    }
    return false;
  }
 
  /**
   * Add a chunk to the chunk list which starts at [address] and is [size] 
   * units large.
   *
   * This will merge the added chunk with its neighboring chunks if possible.
   */
  virtual void Dealloc(AddressType address, SizeType size) {
    if (!size) return;
    
    assert(!ansa::AddWraps<AddressType>(address, size) ||
           (AddressType)(address + size) == 0);
    
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
    
    assert(!after || address + size <= after->start);
    
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
  
  /**
   * Returns the number of free regions.
   */
  size_t GetRegionCount() {
    size_t count = 0;
    FreeRegion * region = firstRegion;
    while (region) {
      ++count;
      region = region->next;
    }
    return count;
  }
  
  /**
   * The structure which is used to represent a region of memory.
   *
   * All allocations that [FreeList] needs from its allocator will be of size
   * `sizeof(FreeRegion)`.
   */
  struct FreeRegion {
    FreeRegion * next;
    AddressType start;
    SizeType size;
  };

protected:  
  FreeRegion * firstRegion = nullptr;
  Allocator<uintptr_t, size_t> & allocator;
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