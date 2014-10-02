#ifndef __ANALLOC2_GENERIC_SCALER_HPP__
#define __ANALLOC2_GENERIC_SCALER_HPP__

#include <cassert>

namespace analloc {

/**
 * Wrap an allocator of class [T] and scale its address space by a constant
 * integer.
 *
 * This is useful when an allocator is being used to allocate virtual memory.
 * Single units in the allocator may represent a larger amount of memory (i.e.
 * 32-bytes), so a [GenericScaler] can be used.
 */
template <class T>
class GenericScaler : public T {
public:
  using typename T::AddressType;
  using typename T::SizeType;
  
  /**
   * Create a [GenericScaler] which multiplies addresses by [scale] before
   * returning them from the [Alloc] method. Conversely, addresses passed to
   * [Dealloc] will be divided by [scale] before being passed to the
   * superclass's implementation.
   */
  template <typename... Args>
  GenericScaler(SizeType _scale, Args... args)
      : T(args...), scale(_scale) {}
  
  virtual bool Alloc(AddressType & output, SizeType size) {
    SizeType scaledSize = size / scale;
    if (size % scale) ++scaledSize;
    if (!T::Alloc(output, scaledSize)) {
      return false;
    }
    output *= scale;
    return true;
  }
  
  virtual void Dealloc(AddressType addr, SizeType size) {
    assert(!(addr % scale));
    T::Dealloc(addr / scale, size * scale);
  }
  
protected:
  SizeType scale;
};

}

#endif
