#ifndef __ANALLOC2_CHUNKED_FREE_LIST_ALLOCATOR_HPP__
#define __ANALLOC2_CHUNKED_FREE_LIST_ALLOCATOR_HPP__

#include "free-list-allocator.hpp"

namespace analloc {

/**
 * A [FreeListAllocator] which forces a natural alignment.  This is useful when
 * you are implementing a back-end for `new` and `delete`.
 */
template <typename AddressType, typename SizeType>
class ChunkedFreeListAllocator
    : public FreeListAllocator<AddressType, SizeType> {
public:
  typedef FreeListAllocator<AddressType, SizeType> super;
  
  using typename super::FailureHandler;
  
  ChunkedFreeListAllocator(size_t _chunkSize, VirtualAllocator & anAlloc,
                           FailureHandler onAllocFail)
      : super(anAlloc, onAllocFail), chunkSize(_chunkSize) {}
  
  virtual bool Alloc(AddressType & addressOut, SizeType size) {
    return super::Alloc(addressOut, ansa::Align(size, chunkSize));
  }
  
  virtual void Dealloc(AddressType address, SizeType size) {
    assert(ansa::IsAligned(address, chunkSize));
    super::Dealloc(address, ansa::Align(size, chunkSize));
  }
  
  inline SizeType GetChunkSize() {
    return chunkSize;
  }
  
protected:
  SizeType chunkSize;
  
  template <typename T>
  friend class AllocatorVirtualizer;
  
  ChunkedFreeListAllocator(size_t _chunkSize, VirtualAllocator * anAlloc,
                           FailureHandler onAllocFail)
      : super(*anAlloc, onAllocFail), chunkSize(_chunkSize) {}
};

}

#endif
