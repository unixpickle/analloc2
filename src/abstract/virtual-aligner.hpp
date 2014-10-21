#ifndef __ANALLOC2_VIRTUAL_ALIGNER_HPP__
#define __ANALLOC2_VIRTUAL_ALIGNER_HPP__

#include "virtual-allocator.hpp"
#include "aligner.hpp"

namespace analloc {

/**
 * A virtual memory allocator which includes the functionality of realloc(),
 * posix_memalign(), and free().
 */
class VirtualAligner
    : public virtual VirtualAllocator,
      public virtual Aligner<uintptr_t, size_t> {
};

}

#endif
