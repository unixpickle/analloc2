#ifndef __ANALLOC2_DEPENDENT_BUFFERED_STACK_HPP__
#define __ANALLOC2_DEPENDENT_BUFFERED_STACK_HPP__

#include "buffered-stack.hpp"

namespace analloc {

/**
 * A [BufferedStack] whose source allocator is determined after initialization.
 *
 * This is useful when the [BufferedStack]'s source allocator depends on the
 * [BufferedStack].
 */
template <size_t Capacity, typename AddressType,
          typename SizeType = AddressType>
class DependentBufferedStack
    : public BufferedStack<Capacity, AddressType, SizeType> {
public:
  typedef BufferedStack<Capacity, AddressType, SizeType> super;
  using typename super::SourceType;
  using typename super::OverflowHandler;
  
  DependentBufferedStack(size_t softMinimum, size_t softMaximum,
                         SizeType objectSize, OverflowHandler overflowHandler)
      : super(softMinimum, softMaximum, objectSize, overflowHandler) {}
  
  inline void SetSource(SourceType * _source) {
    this->source = _source;
  }
  
  inline SourceType * GetSource() {
    return this->source;
  }
};

}

#endif
