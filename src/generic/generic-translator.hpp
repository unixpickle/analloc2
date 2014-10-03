#ifndef __ANALLOC2_GENERIC_TRANSLATOR_HPP__
#define __ANALLOC2_GENERIC_TRANSLATOR_HPP__

#include <cassert>

namespace analloc {

/**
 * Wrap an allocator of class [T] and scale its address space by a constant
 * integer and shift it.
 *
 * This is useful when an allocator is being used to allocate virtual memory.
 * Single units in the allocator may represent a larger amount of memory (i.e.
 * 32-bytes), so a [GenericTranslator] can be used to scale those units by a
 * constant factor. Additionally, the allocator may begin its address space at
 * 0, while most memory regions do not begin at 0. A [GenericTranslator] can be
 * used to address this issue.
 */
template <class T>
class GenericTranslator : public T {
public:
  using typename T::AddressType;
  using typename T::SizeType;
  
  /**
   * Create a [GenericTranslator] which multiplies addresses by [scale] before
   * returning them from the [Alloc] method. Conversely, addresses passed to
   * [Dealloc] will be divided by [scale] before being passed to the
   * superclass's implementation.
   */
  template <typename... Args>
  GenericTranslator(SizeType _scale, AddressType _offset, Args... args)
      : T(args...), scale(_scale), offset(_offset) {}
  
  virtual bool Alloc(AddressType & output, SizeType size) {
    SizeType scaledSize = size / scale;
    if (size % scale) ++scaledSize;
    if (!T::Alloc(output, scaledSize)) {
      return false;
    }
    output = (output * scale) + offset;
    return true;
  }
  
  virtual void Dealloc(AddressType addr, SizeType size) {
    assert(!((addr - offset) % scale));
    T::Dealloc((addr - offset) / scale, size * scale);
  }
  
  inline SizeType GetScale() {
    return scale;
  }
  
  inline AddressType GetOffset() {
    return offset;
  }
  
private:
  SizeType scale;
  AddressType offset;
};

}

#endif
