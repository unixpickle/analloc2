#ifndef __ANALLOC2_VIRTUAL_FREE_LIST_HPP__
#define __ANALLOC2_VIRTUAL_FREE_LIST_HPP__

#include "chunked-free-list.hpp"
#include "../wrappers/aligner-virtualizer.hpp"

namespace analloc {

class VirtualFreeList
    : public AlignerVirtualizer<ChunkedFreeList<uintptr_t, size_t> > {
public:
  typedef AlignerVirtualizer<ChunkedFreeList<uintptr_t, size_t> > super;
  typedef ChunkedFreeList<uintptr_t, size_t>::FailureHandler FailureHandler;
  typedef FreeList<uintptr_t, size_t>::FreeRegion FreeRegion;
  
  /**
   * Create an empty [VirtualFreeList].
   *
   * The passed [chunkSize] will be used not only to align buffers of memory
   * which are returned, but also the memory headers that precede them. This
   * means that you should think twice before passing a [chunkSize] which is
   * greater than `sizeof(size_t)`.
   */
  VirtualFreeList(size_t chunkSize, Allocator<uintptr_t, size_t> & allocator,
                  FailureHandler failure)
      : super(chunkSize, chunkSize, &allocator, failure) {}

  /**
   * Returns the number of free regions.
   */
  inline size_t GetRegionCount() {
    return this->wrapped.GetRegionCount();
  }
};

}

#endif
