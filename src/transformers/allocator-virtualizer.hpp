#ifndef __ANALLOC2_ALLOCATOR_VIRTUALIZER_HPP__
#define __ANALLOC2_ALLOCATOR_VIRTUALIZER_HPP__

#include "allocator-scaler.hpp"
#include "../abstract/virtual-allocator.hpp"
#include <ansa/cstring>

namespace analloc {

/**
 * Provides a simple wrapper around an [AllocatorScaler] providing full
 * [VirtualAllocator] functionality using a small memory header.
 */
template <class T, size_t HeaderPadding = 0>
class AllocatorVirtualizer : public AllocatorScaler<uintptr_t, size_t, T>,
                             public VirtualAllocator {
private:
  struct Header {
    size_t size;
    uint8_t padding[HeaderPadding];
  };

public:
  typedef AllocatorScaler<uintptr_t, size_t, T> super;
    
  template <typename... Args>
  AllocatorVirtualizer(uintptr_t start, uintptr_t scale, Args... args)
      : super(start, scale, args...) {}
  
  virtual bool Alloc(uintptr_t & out, size_t size) {
    uintptr_t buffer;
    if (!super::Alloc(buffer, size + sizeof(Header))) {
      return false;
    }
    Header * header = (Header *)buffer;
    header->size = size;
    out = buffer + sizeof(Header);
    return true;
  }
  
  virtual void Dealloc(uintptr_t pointer, size_t size) {
    assert(((Header *)(pointer - sizeof(Header)))->size == size);
    super::Dealloc(pointer - sizeof(Header), size + sizeof(Header));
  }
  
  virtual bool Realloc(uintptr_t & address, size_t size) {
    Header * oldHeader = (Header *)(address - sizeof(Header));
    size_t oldSize = oldHeader->size;
    
    uintptr_t newBuf;
    if (!Alloc(newBuf, size + sizeof(Header))) {
      return false;
    }
    
    ansa::Memcpy((void *)(newBuf + sizeof(Header)), (void *)address,
                 oldSize < size ? oldSize : size);
    super::Dealloc(address - sizeof(Header), oldSize + sizeof(Header));
    address = newBuf + sizeof(Header);
    return true;
  }
  
  virtual void Free(uintptr_t pointer) {
    Header * header = (Header *)(pointer - sizeof(Header));
    super::Dealloc(pointer - sizeof(Header), header->size + sizeof(Header));
  }
};

}

#endif
