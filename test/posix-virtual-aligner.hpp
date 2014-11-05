#ifndef __TEST_POSIX_VIRTUAL_ALIGNER_HPP__
#define __TEST_POSIX_VIRTUAL_ALIGNER_HPP__

#include <analloc2/abstract>
#include <ansa/cstring>
#include <ansa/math>
#include <cassert>
#include <cstdlib>

class PosixVirtualAligner : public analloc::VirtualOffsetAligner {
public:
  struct Header {
    void * pointer;
    size_t size;
  };
  
  virtual bool Alloc(uintptr_t & out, size_t size) {
    void * buf = malloc(size + sizeof(Header));
    if (!buf) return false;
    
    Header * hdr = (Header *)buf;
    hdr->pointer = buf;
    hdr->size = size;
    
    ++allocCount;
    out = (uintptr_t)buf + sizeof(Header);
    return true;
  }
  
  virtual void Dealloc(uintptr_t ptr, size_t) {
    Free(ptr);
  }
  
  virtual bool Realloc(uintptr_t & address, size_t newSize) {
    uintptr_t newAddr;
    if (!Alloc(newAddr, newSize)) {
      return false;
    }
    Header * oldHeader = (Header *)(address - sizeof(Header));
    size_t size = ansa::Min(newSize, oldHeader->size);
    ansa::Memcpy((void *)newAddr, (void *)address, size);
    Free(address);
    address = newAddr;
    return true;
  }
  
  virtual void Free(uintptr_t ptr) {
    --allocCount;
    Header * hdr = (Header *)(ptr - sizeof(Header));
    free((void *)hdr->pointer);
  }
  
  virtual bool OffsetAlign(uintptr_t & output, uintptr_t align,
                           uintptr_t offset, size_t size) {
    // While this implementation is quite wasteful, I could not think of a 
    // nicer way to do it for the general case.
    size_t objectSize = ansa::Max(size, (size_t)align);
    size_t actualSize = (objectSize + sizeof(Header)) * 2;
    void * buffer = malloc(actualSize);
    if (!buffer) {
      return false;
    }
    // "Correct" the output by offseting it.
    output = (uintptr_t)buffer;
    uintptr_t misalign = (output + offset) % align;
    uintptr_t correction = align - misalign;
    if (correction < sizeof(Header)) {
      correction += align;
    }
    output += correction;
    // Create the header
    Header * header = (Header *)(output - sizeof(Header));
    header->pointer = buffer;
    header->size = size;
    ++allocCount;
    return true;
  }
  
  inline size_t GetAllocCount() {
    return allocCount;
  }
  
private:
  size_t allocCount = 0;
};

#endif
