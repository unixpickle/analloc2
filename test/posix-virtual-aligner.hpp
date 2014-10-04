#ifndef __TEST_POSIX_VIRTUAL_ALIGNER_HPP__
#define __TEST_POSIX_VIRTUAL_ALIGNER_HPP__

#include <analloc2>

class PosixVirtualAligner : public analloc::VirtualAligner {
public:
  virtual bool Alloc(uintptr_t & out, size_t size);
  virtual void Dealloc(uintptr_t ptr, size_t);
  virtual bool Realloc(uintptr_t & address, size_t newSize);
  virtual void Free(uintptr_t ptr);
  virtual bool Align(uintptr_t & output, uintptr_t align, size_t size);
  
  inline size_t GetAllocCount() {
    return allocCount;
  }
  
private:
  size_t allocCount = 0;
};

#endif
