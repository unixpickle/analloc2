#ifndef __ANALLOC2_VIRTUAL_ALLOCATOR_HPP__
#define __ANALLOC2_VIRTUAL_ALLOCATOR_HPP__

#include "allocator.hpp"
#include <cstdint>
#include <cstddef>

namespace analloc {

/**
 * A virtual memory allocator which includes the functionality of realloc() and
 * free().
 */
class VirtualAllocator : public virtual Allocator<uintptr_t, size_t> {
public:
  /**
   * Reallocate the memory located at [address] to a new [size].
   *
   * The resized pointer will be returned through the [address] argument. The
   * new memory region must be at least [size] bytes.
   *
   * A return value of `true` indicates that the reallocation succeeded. If the
   * return value is `false`, the memory located at [address] has not been
   * affected.
   */
  virtual bool Realloc(uintptr_t & address, size_t size) = 0;
  
  /**
   * Deallocate a memory region without knowing its size.
   */
  virtual void Dealloc(uintptr_t) = 0;
};

}

#endif
