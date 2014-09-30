#ifndef __ANALLOC2_FREE_LIST_ALIGNER_HPP__
#define __ANALLOC2_FREE_LIST_ALIGNER_HPP__

#include "free-list-allocator.hpp"

namespace analloc {

template <typename AddressType, typename SizeType = AddressType>
class FreeListAligner : public FreeListAllocator<AddressType, SizeType> {
public:
  using typename FreeListAllocator<AddressType, SizeType>::FreeRegion;
  
  virtual bool Align(AddressType & out, AddressType align, SizeType size) {
    FreeRegion * reg = this->firstRegion;
    while (reg) {
      // Compute the offset in this region to align it properly.
      SizeType offset = 0;
      if (reg->start % align) {
        offset = (SizeType)(align - (reg->start % align));
        if (offset >= reg->size) {
          // Aligning an address in this region pushes the address out of
          // bounds.
          continue;
        }
      }
      // Check for the five cases:
      // - there isn't enough room in this region
      // - there is exactly enough room in this region and offset=0
      // - there is more than enough room in this region and offset=0
      // - there is just enough room in this region with offset!=0
      // - there is more than enough room in this region with offset!=0
      if (reg->size - offset < size) {
        reg = reg->next;
      } else if (offset == 0 && size == reg->size) {
        // Remove the region from the list
        if (!reg->last) {
          this->firstRegion = reg->next;
        } else {
          reg->last->next = reg->next;
        }
        if (reg->next) {
          reg->next->last = reg->last;
        }
        out = reg->start;
        this->allocator.Dealloc((uintptr_t)reg, sizeof(FreeRegion));
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
        uintptr_t ptr;
        if (!this->allocator.Alloc(ptr, sizeof(FreeRegion))) {
          failureHandler(this);
          return false;
        }
        FreeRegion * newReg = (FreeRegion *)ptr;
        newReg->start = reg->start + offset + size;
        newReg->size = reg->size - (offset + size);
        newReg->last = reg;
        newReg->next = reg->next;
        reg->next = newReg;
        reg->size = offset;
        if (newReg->next) {
          newReg->next->last = newReg;
        }
      }
    }
    return false;
  }
};

}

#endif