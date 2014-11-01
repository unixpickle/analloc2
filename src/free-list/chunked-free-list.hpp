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
      : super(anAlloc, onAllocFail), chunkSize(_chunkSize) {
    assert(ansa::IsPowerOf2(chunkSize));
  }
  
  virtual bool Alloc(AddressType & addressOut, SizeType size) {
    return super::Alloc(addressOut, ansa::Align2(size, chunkSize));
  }
  
  virtual void Dealloc(AddressType address, SizeType size) {
    assert(ansa::IsAligned2(address, chunkSize));
    super::Dealloc(address, ansa::Align2(size, chunkSize));
  }
  
  virtual bool OffsetAlign(AddressType & addressOut, AddressType align,
                           AddressType offset, SizeType size) {
    if (!ansa::IsAligned2(offset, chunkSize)) {
      return false;
    } else if (align <= chunkSize) {
      return Alloc(addressOut, size);
    } else {
      return super::OffsetAlign(addressOut, align, offset,
                                ansa::Align2(size, chunkSize));
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
