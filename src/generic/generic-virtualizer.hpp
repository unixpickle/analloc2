#ifndef __ANALLOC2_GENERIC_VIRTUALIZER_HPP__
#define __ANALLOC2_GENERIC_VIRTUALIZER_HPP__

#include "../abstract/virtual-allocator.hpp"
#include <ansa/cstring>

namespace analloc {

/**
 * Provides a simple wrapper around an [Allocator] providing full
 * [VirtualAllocator] functionality using a small memory header.
 */
template <class T, size_t HeaderPadding = 0>
class GenericVirtualizer : public T,
                           public virtual VirtualAllocator {
protected:
  struct Header {
    size_t size;
    uint8_t padding[HeaderPadding];
  };

public:
  template <typename... Args>
  GenericVirtualizer(Args... args) : T(args...) {}
  
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
    assert(((Header *)(pointer - sizeof(Header)))->size == size);
    // Deallocate the original pointer by subtracting sizeof(Header) to the
    // address and adding it to the length.
    T::Dealloc(pointer - sizeof(Header), size + sizeof(Header));
  }
  
  virtual bool Realloc(uintptr_t & address, size_t size) {
    // Get the information for the current buffer.
    Header * oldHeader = (Header *)(address - sizeof(Header));
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
    Header * header = (Header *)(pointer - sizeof(Header));
    // Free the entire buffer, including the header.
    T::Dealloc(pointer - sizeof(Header), header->size + sizeof(Header));
  }
};

}

#endif
