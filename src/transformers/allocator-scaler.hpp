#ifndef __ANALLOC2_ALLOCATOR_SCALER_HPP__
#define __ANALLOC2_ALLOCATOR_SCALER_HPP__

#include <cassert>

namespace analloc {

/**
 * Turn an allocator of class [T] into another allocator which scales the
 * addresses it returns.
 */
template <class T, typename AddressType, typename SizeType = AddressType>
class AllocatorScaler : public T {
public:
  template <typename... Args>
  AllocatorScaler(AddressType _start, AddressType _scale, Args... args)
      : T(args...), scalerStart(_start), scalerMultiple(_scale) {}
  
  virtual bool Alloc(AddressType & output, SizeType size) {
    if (!T::Alloc(output, ScaleSize(size))) {
      return false;
    }
    output = ScaleAddress(output);
    return true;
  }
  
  virtual void Dealloc(AddressType addr, SizeType size) {
    T::Dealloc(ScaleAddress(addr), ScaleSize(size));
  }
  
  inline SizeType ScaleSize(SizeType size) {
    SizeType newSize = (SizeType)(size / scalerMultiple);
    if (size % scalerMultiple) {
      return newSize + 1;
    } else {
      return newSize;
    }
  }
  
  inline AddressType ScaleAddress(AddressType addr) {
    assert(addr >= scalerStart);
    return ((addr - scalerStart) * scalerMultiple) + scalerStart;
  }
  
protected:
  AddressType scalerStart;
  AddressType scalerMultiple;
};

}

#endif
