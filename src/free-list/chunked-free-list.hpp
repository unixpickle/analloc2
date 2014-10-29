#ifndef __ANALLOC2_CHUNKED_FREE_LIST_HPP__
#define __ANALLOC2_CHUNKED_FREE_LIST_HPP__

#include "free-list.hpp"

namespace analloc {

/**
 * A [FreeList] which forces a natural alignment. This is useful when you are
 * implementing a back-end for `malloc`, `posix_memalign`, and `free`.
 */
template <typename AddressType, typename SizeType = AddressType>
class ChunkedFreeList : public FreeList<AddressType, SizeType> {
public:
  typedef FreeList<AddressType, SizeType> super;
  
  using typename super::FailureHandler;
  
  ChunkedFreeList(size_t _chunkSize, VirtualAllocator & anAlloc,
                  FailureHandler onAllocFail)
      : super(anAlloc, onAllocFail), chunkSize(_chunkSize) {}
  
  virtual bool Alloc(AddressType & addressOut, SizeType size) {
    return super::Alloc(addressOut, ansa::Align(size, chunkSize));
  }
  
  virtual void Dealloc(AddressType address, SizeType size) {
    assert(ansa::IsAligned(address, chunkSize));
    super::Dealloc(address, ansa::Align(size, chunkSize));
  }
  
  virtual bool OffsetAlign(AddressType & addressOut, AddressType align,
                           AddressType offset, SizeType size) {
    if (!ansa::IsAligned(offset, chunkSize)) {
      return false;
    } else if (align <= chunkSize) {
      return Alloc(addressOut, size);
    } else {
      return super::OffsetAlign(addressOut, align, offset,
                                ansa::Align(size, chunkSize));
    }
  }
  
  inline SizeType GetChunkSize() {
    return chunkSize;
  }
  
protected:
  SizeType chunkSize;
  
  template <typename T>
  friend class AllocatorVirtualizer;
  
  ChunkedFreeList(size_t _chunkSize, VirtualAllocator * anAlloc,
                  FailureHandler onAllocFail)
      : super(*anAlloc, onAllocFail), chunkSize(_chunkSize) {}
};

}

#endif
