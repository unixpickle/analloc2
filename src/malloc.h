#ifndef __ANALLOC2_MALLOC_H__
#define __ANALLOC2_MALLOC_H__

#include "allocator.h"
#include "utility.h"

namespace ANAlloc {

/**
 * This is a subclass of an Allocator which acts as a self-contained memory
 * allocator for general use. It requires that you create a contiguous region
 * of memory, and then it installs itself within this region by allocating
 * room for its binary tree data.
 */
template <class T>
class Malloc : public Allocator<T> {
protected:
  void * start; // region start
  size_t length; // region length in bytes
  size_t pageSize; // size of lowest-level nodes
  int pageSizeLog;
  T treeObject;
  
public:
  /**
   * Create a new allocator.
   * @param base The start of the memory region to use.
   * @param pageSize The minimum number of bytes to allow the user to allocate.
   * @param usedBytes The number of bytes at the beginning of the buffer which
   * have already been reserved and should not be touched.
   * @param total The total number of bytes in the buffer.
   */
  Malloc(void * base, size_t pageSize, size_t usedBytes, size_t total);
  
  /**
   * Attempts to allocate a buffer of at least `size`. If the operation fails,
   * NULL is returned.
   */
  void * AllocBuf(size_t size);
  
  /**
   * Free a buffer which was returned with AllocBuf().
   */
  void FreeBuf(void * buff);
  
  /**
   * Returns `true` if the buffer is contained within this allocator.
   */
  bool OwnsPointer(void * ptr);
  
};

template <class T>
Malloc<T>::Malloc(void * base, size_t page, size_t used, size_t total)
  : treeObject(Log2Floor(total / page), (uint8_t *)base + used) {
  this->tree = &treeObject;
  start = base;
  length = total;
  pageSize = page;
  pageSizeLog = Log2Floor(pageSize);
  assert((1L << pageSizeLog) == pageSize);
  
  size_t realUsed = used + T::MemorySize(treeObject.Depth());
  if (realUsed % pageSize) {
    realUsed += pageSize - (realUsed % pageSize);
  }
  Path p;
  this->Alloc(0, p);
  this->Reserve(p, 0, realUsed / pageSize);
}

template <class T>
void * Malloc<T>::AllocBuf(size_t size) {
  if (size > (pageSize << (treeObject.Depth() - 2))) {
    return NULL;
  }
  
  int power = Log2Ceil(size) - pageSizeLog;
  if (power < 0) power = 0;
  
  int depth = treeObject.Depth() - power - 1;
  Path p = 0;
  bool res = this->Alloc(depth, p);
  if (!res) return NULL;
  return (void *)(PathIndex(p) * (pageSize << power));
}

template <class T>
void Malloc<T>::FreeBuf(void * buff) {
  uintptr_t baseIndex = (uintptr_t)buff - (uintptr_t)start;
  baseIndex /= pageSize;
  
  Path p;
  bool res = this->Find(baseIndex, p);
  assert(res);
  this->Free(p);
}

template <class T>
bool Malloc<T>::OwnsPointer(void * ptr) {
  if (ptr < start) return false;
  if ((uintptr_t)ptr >= (uintptr_t)start + length) return false;
  return true;
}

}

#endif
