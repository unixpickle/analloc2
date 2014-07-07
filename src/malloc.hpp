#ifndef __ANALLOC2_MALLOC_H__
#define __ANALLOC2_MALLOC_H__

#include "tree/tree.hpp"
#include "log.hpp"
#include <new>

namespace ANAlloc {

/**
 * This lightweight template wraps the tree of your choice to provide a simple
 * linear address allocator.
 */
class Malloc {  
public:
  template <class T>
  static Malloc * WrapRegion(void * start, size_t length, int psLog);
  
  Malloc(void * start, Tree & tree, int psLog);
  
  /**
   * Attempts to allocate a buffer of at least `size` bytes. If the operation
   * fails, NULL is returned.
   */
  void * Alloc(size_t size);
  
  /**
   * Attempts to allocate memory of a certain size, aligned by a certain size.
   * @discussion The alignment must be a power of two.
   */
  void * Align(size_t size, size_t align);
  
  /**
   * Free a buffer which was returned with Alloc().
   */
  void Free(void * buff);
  
  /**
   * Returns `true` if the buffer is contained within this allocator.
   */
  bool OwnsPointer(void * ptr);
  
protected:
  void * start;
  uint64_t length;
  
  T & tree;
  int psLog;
  
  int DepthForSize(size_t size);
};

template <class T>
Malloc * Malloc::WrapRegion(void * start, size_t length, int psLog) {
  int sizeLog = Log2Floor(length);
  int maxDepth = sizeLog - psLog;
  if (maxDepth < 0) return NULL;
  
  size_t useLength = sizeof(T) + sizeof(Malloc) + T::MemorySize(maxDepth + 1);
  if (useLength > length) return NULL;
  
  T * tree = (T *)base;
  Malloc * result = (Malloc *)((uint8_t *)base + sizeof(T));
  uint8_t * bmBuffer = (uint8_t *)base + sizeof(T) + sizeof(Malloc);
  
  // create the tree
  new(tree) T(maxDepth + 1, bmBuffer);
  
  // carve out the beginning of the buffer
  uint64_t carvePageCount = useLength >> psLog;
  if (useLength % (1UL << psLog)) {
    ++carvePageCount;
  }
  tree->SetType(Path::Root(), NodeTypeData);
  tree->Carve(Path::Root(), 0, carvePageCount);
  
  // create the actually Malloc object
  new(result) Malloc(start, *tree, psLog);
  return result;
}

}

#endif
