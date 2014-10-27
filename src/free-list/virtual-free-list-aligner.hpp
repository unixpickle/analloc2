#ifndef __ANALLOC2_VIRTUAL_FREE_LIST_ALIGNER_HPP__
#define __ANALLOC2_VIRTUAL_FREE_LIST_ALIGNER_HPP__

#include "chunked-free-list-aligner.hpp"
#include "../wrappers/aligner-virtualizer.hpp"

namespace analloc {

class VirtualFreeListAligner
    : public AlignerVirtualizer<
          ChunkedFreeListAligner<uintptr_t, size_t>
      > {
public:
  typedef AlignerVirtualizer<ChunkedFreeListAligner<uintptr_t, size_t> >
      super;
  typedef ChunkedFreeListAligner<uintptr_t, size_t>::FailureHandler
      FailureHandler;
  
 /**
  * Create an empty [VirtualFreeListAligner].
  *
  * The passed [chunkSize] will be used not only to align buffers of memory
  * which are returned, but also the memory headers that precede them. This
  * means that you should think twice before passing a [chunkSize] which is
  * greater than `sizeof(size_t)`.
  */
 VirtualFreeListAligner(size_t chunkSize, VirtualAllocator & allocator,
                        FailureHandler failure)
     : super(chunkSize, chunkSize, allocator, failure) {}
};

}

#endif
