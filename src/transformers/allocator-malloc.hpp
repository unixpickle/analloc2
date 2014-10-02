#ifndef __ANALLOC2_ALLOCATOR_MALLOC_HPP__
#define __ANALLOC2_ALLOCATOR_MALLOC_HPP__

#include "allocator-scaler.hpp"

namespace analloc {

template <class T>
class AllocatorMalloc : public virtual AllocatorScaler<uintptr_t, size_t, T> {
public:
  template <typename... Args>
  AllocatorScaler(uintptr_t start, uintptr_t scale, Args... args)
      : AllocatorScaler(start, scale, args) {}
};

}

#endif
