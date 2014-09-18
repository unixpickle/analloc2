#ifndef __ANALLOC2_ALLOCATOR_HPP__
#define __ANALLOC2_ALLOCATOR_HPP__

namespace analloc {

/**
 * This is an allocator in its most basic form.
 */
template <typename AddressType, typename SizeType = AddressType>
class Allocator {
public:
  /**
   * Allocate [size] units and return the beginning of the allocated region via
   * the [addressOut] argument.
   *
   * A return value of `true` indicates a successful allocation. A return value
   * of `false` indicates an error, but does not imply that no region of [size]
   * units exists. For example, allocating memory from a buddy allocator may
   * fail in the case where an available memory region is not properly aligned.
   */
  virtual bool Alloc(AddressType & addressOut, SizeType size) = 0;
  
  /**
   * Deallocate a region that was allocated with [Alloc].
   *
   * Some allocators do not keep track of the size of an allocated region of
   * memory. Thus, you must pass the size of the memory region via the [size]
   * argument. Some allocators may provide a version of [Dealloc] that does not
   * require a [size] argument.
   */
  virtual void Dealloc(AddressType address, SizeType size) = 0;
};

}

#endif
