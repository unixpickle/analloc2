#ifndef __ANALLOC2_VIRTUAL_FREE_LIST_ALLOCATOR_HPP__
#define __ANALLOC2_VIRTUAL_FREE_LIST_ALLOCATOR_HPP__

#include "chunked-free-list-allocator.hpp"
#include "../wrappers/allocator-virtualizer.hpp"

namespace analloc {

/**
 * A virtual allocator which wraps [ChunkedFreeListAllocator].
 */
class VirtualFreeListAllocator
    : public AllocatorVirtualizer<
          ChunkedFreeListAllocator<uintptr_t, size_t>
      > {
public:
  typedef AllocatorVirtualizer<ChunkedFreeListAllocator<uintptr_t, size_t> >
      super;
  typedef ChunkedFreeListAllocator<uintptr_t, size_t>::FailureHandler
      FailureHandler;
  
  /**
   * Create an empty [VirtualFreeListAllocator].
   *
   * The passed [chunkSize] will be used not only to align buffers of memory
   * which are returned, but also the memory headers that precede them. This
   * means that you should think twice before passing a [chunkSize] which is
   * greater than `sizeof(size_t)`.
   */
  VirtualFreeListAllocator(size_t chunkSize, VirtualAllocator & allocator,
                           FailureHandler failure)
      : super(chunkSize, chunkSize, allocator, failure) {}
};

}

#endif
