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
        out = reg->start;
        Remove(reg);
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
        InsertAfter(reg, reg->start + offset + size,
                    reg->size - (offset + size));
        reg->size = offset;
      }
    }
    return false;
  }
};

}

#endif