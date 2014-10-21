#ifndef __ANALLOC2_VIRTUAL_OFFSET_ALIGNER_HPP__
#define __ANALLOC2_VIRTUAL_OFFSET_ALIGNER_HPP__

#include "virtual-aligner.hpp"
#include "offset-aligner.hpp"

namespace analloc {

class VirtualOffsetAligner
    : public virtual VirtualAligner,
      public virtual OffsetAligner<uintptr_t, size_t> {
};

}

#endif
