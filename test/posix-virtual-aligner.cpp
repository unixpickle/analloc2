#include "posix-virtual-aligner.hpp"
#include <stdlib.h>

bool PosixVirtualAligner::Alloc(uintptr_t & out, size_t size) {
  void * buf = malloc(size);
  if (!buf) return false;
  ++allocCount;
  out = (uintptr_t)buf;
  return true;
}

void PosixVirtualAligner::Dealloc(uintptr_t ptr, size_t) {
  --allocCount;
  free((void *)ptr);
}

bool PosixVirtualAligner::Realloc(uintptr_t & address, size_t newSize) {
  void * newBuf = realloc((void *)address, newSize);
  if (newBuf) {
    address = (uintptr_t)newBuf;
    return true;
  }
  return false;
}

void PosixVirtualAligner::Free(uintptr_t ptr) {
  free((void *)ptr);
  --allocCount;
}

bool PosixVirtualAligner::Align(uintptr_t & output, uintptr_t align,
                                size_t size) {
  if ((uintptr_t)((size_t)align) != align) {
    // posix_memalign() uses a size_t for alignment instead of a uintptr_t. I
    // find this potentially limiting, so I use uintptr_t.
    return false;
  }
  void * result;
  if (posix_memalign(&result, (size_t)align, size)) {
    return false;
  }
  ++allocCount;
  output = (uintptr_t)result;
  return true;
}
