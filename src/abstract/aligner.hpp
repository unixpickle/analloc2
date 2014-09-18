#ifndef __ANALLOC2_ALIGNER_HPP__
#define __ANALLOC2_ALIGNER_HPP__

#include "allocator.hpp"

namespace analloc {

template <typename AddressType, typename SizeType = AddressType>
class Aligner : public Allocator<AddressType, SizeType> {
public:
 /**
  * Allocate [size] units and return the beginning of the allocated region via
  * the [addressOut] argument. The returned region will be aligned by [align]
  * units.
  *
  * The specified [align] argument must be a power of 2. The returned address
  * will meet the criteria: `addressOut & (align - 1) == 0`. An [align] value
  * of `0` will be treated as an alignment of `1`.
  *
  * Memory allocated via [Align] can be deallocated with the [Dealloc] method.
  *
  * A return value of `true` indicates a successful allocation. A return value
  * of `false` indicates an error, but does not imply that no aligned region
  * of [size] units exists. A failure could simply mean that the algorithm
  * behind this allocator is not capable of utilizing a certain aligned region
  * of memory.
  */
 virtual bool Align(AddressType & addressOut, AddressType align,
                    SizeType size) = 0;
};

}

#endif
