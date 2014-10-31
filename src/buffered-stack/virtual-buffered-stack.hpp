#ifndef __ANALLOC2_VIRTUAL_BUFFERED_STACK_HPP__
#define __ANALLOC2_VIRTUAL_BUFFERED_STACK_HPP__

#include "buffered-stack.hpp"
#include "../abstract/virtual-allocator.hpp"

namespace analloc {

/**
 * A [BufferedStack] which includes [Realloc] and [Free] methods.
 */
template <size_t Capacity>
class VirtualBufferedStack
    : public BufferedStack<Capacity, uintptr_t, size_t>,
      public virtual VirtualAllocator {
public:
  typedef BufferedStack<Capacity, uintptr_t, size_t> super;
  
  using typename super::OverflowHandler;
  using typename super::SourceType;
  
  VirtualBufferedStack(SourceType & source, size_t softMinimum,
                       size_t softMaximum, SizeType objectSize,
                       OverflowHandler overflowHandler)
      : super(source, softMinimum, softMaximum, objectSize, overflowHandler) {}
  
  virtual bool Realloc(uintptr_t &, size_t newSize) {
    return newSize <= this->objectSize;
  }
  
  virtual void Free(uintptr_t ptr) {
    this->Dealloc(ptr, this->objectSize);
  }
};

}

#endif
