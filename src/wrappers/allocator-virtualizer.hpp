#ifndef __ANALLOC2_ALLOCATOR_VIRTUALIZER_HPP__
#define __ANALLOC2_ALLOCATOR_VIRTUALIZER_HPP__

#include "../abstract/virtual-allocator.hpp"
#include <ansa/cstring>

namespace analloc {

/**
 * A simple wrapper for an [Allocator] which provides full [VirtualAllocator]
 * functionality.
 *
 * The [AllocatorVirtualizer] achieves its goal through the use of a small
 * memory header which prefixes every allocated region of memory. You may
 * specify the optional [HeaderPadding] template argument to add bytes to the
 * end of the header struct.
 */
template <class T, size_t HeaderPadding = 0>
class AllocatorVirtualizer : public virtual T,
                             public virtual VirtualAllocator {
public:
  /**
   * The header structure which precedes all returned regions of memory.
   */
  struct Header {
    size_t size;
    uint8_t padding[HeaderPadding];
  };
  
  template <typename... Args>
  AllocatorVirtualizer(Args... args) : T(args...) {}
  
  virtual bool Alloc(uintptr_t & out, size_t size) {
    // We need size + sizeof(Header) bytes in order to store the header
    uintptr_t buffer;
    if (!T::Alloc(buffer, size + sizeof(Header))) {
      return false;
    }
    // Set the size in the header
    Header * header = (Header *)buffer;
    header->size = size;
    // Return the part of the buffer after the header
    out = buffer + sizeof(Header);
    return true;
  }
  
  virtual void Dealloc(uintptr_t pointer, size_t size) {
    // Assert that the [size] is correct
    assert(RegionHeader(pointer)->size == size);
    
    // Deallocate the original pointer by subtracting sizeof(Header) to the
    // address and adding it to the length.
    T::Dealloc(pointer - sizeof(Header), size + sizeof(Header));
  }
  
  /**
   * Allocate a new region of memory of [size] bytes, copy over the memory from
   * [address], deallocate [address], and return the new memory buffer through
   * the [address] argument.
   *
   * Some allocators may be able to improve on this algorithm. For instance,
   * the current region of memory may be directly expandable; in this case,
   * there is no need for a copy operation.
   */
  virtual bool Realloc(uintptr_t & address, size_t size) {
    // Get the information for the current buffer.
    Header * oldHeader = RegionHeader(address);
    size_t oldSize = oldHeader->size;
    
    // Attempt to allocate a new buffer using this classes [Alloc] method.
    uintptr_t newBuf;
    if (!Alloc(newBuf, size)) {
      // If allocation fails, nothing is changed.
      return false;
    }
    
    // Copy memory from the old buffer to the new buffer. This is bounded by
    // the sizes of both the old buffer and the new buffer.
    ansa::Memcpy((void *)newBuf, (void *)address,
                 oldSize < size ? oldSize : size);
    
    // Deallocate the old buffer using our [Dealloc] method.
    Dealloc(address, oldSize);
    
    // Return the new buffer.
    address = newBuf;
    return true;
  }
  
  virtual void Free(uintptr_t pointer) {
    // Get the buffer's header.
    Header * header = RegionHeader(pointer);
    // Free the entire buffer, including the header.
    T::Dealloc(pointer - sizeof(Header), header->size + sizeof(Header));
  }
  
protected:
  inline Header * RegionHeader(uintptr_t pointer) {
    return (Header *)(pointer - sizeof(Header));
  }
};

}

#endif
