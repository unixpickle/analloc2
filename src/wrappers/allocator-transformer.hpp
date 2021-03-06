#ifndef __ANALLOC2_ALLOCATOR_TRANSFORMER_HPP__
#define __ANALLOC2_ALLOCATOR_TRANSFORMER_HPP__

#include "../abstract/allocator.hpp"
#include <cassert>
#include <ansa/math>

namespace analloc {

/**
 * An allocator which scales and translates addresses from an enclosed 
 * allocator [T].
 *
 * To understand why this is useful, imagine using a bitmap allocator to hand
 * out regions of free memory. If one bit in the bitmap corresponds to one byte
 * of virtual memory, the bitmap would consume 1/9th of the entire memory
 * of the system. Furthermore, the first bit in the bitmap would correspond to
 * the first byte of the system's memory, which is probably reserved by the
 * operating system.
 *
 * To address the situation above, you can use an [AllocatorTransformer] to
 * scale addresses so that one bit in the bitmap corresponds to, say, 16 bytes
 * of memory. Furthermore, the [AllocatorTransformer] also translates
 * addresses so that the address space need not begin at `nullptr`.
 */
template <class T>
class AllocatorTransformer
    : public virtual Allocator<typename T::AddressType, typename T::SizeType> {
public:
  typedef typename T::AddressType AddressType;
  typedef typename T::SizeType SizeType;
  
  /**
   * Create an [AllocatorTransformer] instance, passing [args] to [T]'s
   * constructor.
   *
   * The [_scale] argument specifies the scale factor for addresses. The 
   * [_offset] argument indicates how much the address space ought to be 
   * offset. The [Alloc] and [Dealloc] methods detail how these arguments are
   * used.
   */
  template <typename... Args>
  AllocatorTransformer(SizeType _scale, AddressType _offset, Args... args)
      : wrapped(args...), scale(_scale), offset(_offset) {
    assert(scale != 0);
  }
  
  /**
   * Allocate address space from the wrapped allocator and transform the
   * resultant address.
   *
   * The [size] argument is scaled down before being passed to the enclosed
   * allocator.
   *
   * When the wrapped allocator returns an address x, [output] will be set to 
   * (x * scale) + offset.
   */
  virtual bool Alloc(AddressType & output, SizeType size) {
    if (!wrapped.Alloc(output, ScaleSize(size))) {
      return false;
    }
    output = OutputAddress(output);
    return true;
  }
  
  /**
   * Deallocate address space from the wrapped allocator.
   *
   * The [size] argument is scaled down before being passed to the superclass.
   *
   * The address which is passed to the wrapped allocator is calculated by
   * ([addr] - offset) / scale.
   */
  virtual void Dealloc(AddressType addr, SizeType size) {
    assert(!((addr - offset) % scale));
    wrapped.Dealloc((addr - offset) / scale, ScaleSize(size));
  }
  
  /**
   * Get the scale factor which this allocator applies to addresses from its
   * wrapped allocator.
   */
  inline SizeType GetScale() const {
    return scale;
  }
  
  /**
   * Get the offset which this allocator applies to addresses from its
   * wrapped allocator.
   */
  inline AddressType GetOffset() const {
    return offset;
  }
  
protected:
  T wrapped;
  SizeType scale;
  AddressType offset;
  
  /**
   * Scale a size from the outside world to a size which should be passed to
   * the wrapped allocator's methods.
   */
  inline SizeType ScaleSize(SizeType size) {
    return ansa::RoundUpDiv<SizeType>(size, scale);
  }
  
  /**
   * Translate an address from the wrapped allocator to an output address.
   */
  inline AddressType OutputAddress(AddressType address) {
    return (address * scale) + offset;
  }
};

}

#endif
