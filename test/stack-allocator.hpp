#ifndef __TEST_STACK_ALLOCATOR_HPP__
#define __TEST_STACK_ALLOCATOR_HPP__

#include <analloc2/abstract>
#include <cassert>

template <size_t ObjSize>
class StackAllocator : public analloc::VirtualAllocator {
public:
  StackAllocator(size_t _capacity, analloc::VirtualAllocator & _source)
      : source(_source),
        capacity(_capacity),
        buffers(new uintptr_t[_capacity]) {
    for (currentCount = 0; currentCount < capacity; ++currentCount) {
      if (!source.Alloc(buffers[currentCount], ObjSize)) {
        break;
      }
    }
  }
  
  virtual ~StackAllocator() {
    assert(capacity == currentCount);
    for (size_t i = 0; i < capacity; ++i) {
      source.Dealloc(buffers[i], ObjSize);
    }
    delete[] buffers;
  }
  
  virtual bool Alloc(uintptr_t & out, size_t size) {
    if (size > ObjSize) return false;
    if (!currentCount) return false;
    out = buffers[--currentCount];
    return true;
  }
  
  virtual bool Realloc(uintptr_t &, size_t newSize) {
    if (newSize > ObjSize) return false;
    return true;
  }
  
  virtual void Free(uintptr_t ptr) {
    assert(currentCount < capacity);
    buffers[currentCount++] = ptr;
  }
  
  virtual void Dealloc(uintptr_t ptr, size_t size) {
    assert(size <= ObjSize);
    Free(ptr);
  }
  
private:
  analloc::VirtualAllocator & source;
  size_t capacity;
  size_t currentCount;
  uintptr_t * buffers;
};

#endif
