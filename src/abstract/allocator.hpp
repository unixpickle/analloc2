#ifndef __ANALLOC2_ALLOCATOR_HPP__
#define __ANALLOC2_ALLOCATOR_HPP__

#include <ansa/nocopy>
#include <ansa/numeric-info>

namespace analloc {

/**
 * This is an allocator in its most basic form.
 */
template <typename _AddressType, typename _SizeType = _AddressType>
class Allocator : public ansa::NoCopy {
public:
  static_assert(ansa::NumericInfo<_AddressType>::max >= 
                ansa::NumericInfo<_SizeType>::max,
                "AddressType cannot be smaller than SizeType.");
  
  /**
   * The type which can address any individual unit within this allocator.
   *
   * This type must be no smaller than [SizeType].
   */
  typedef _AddressType AddressType;
  
  /**
   * The type which can be used to contain the size of the largest continuous
   * chunk of address space that can be allocated in one piece.
   *
   * This type must be no larger than [AddressType].
   */
  typedef _SizeType SizeType;
  
  /**
   * Since allocators have virtual methods, they should have virtual
   * destructors.
   */
  virtual ~Allocator() {}
  
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
