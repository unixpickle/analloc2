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
  static Malloc * WrapRegion(uint8_t * start, size_t length, int psLog);
  
  Malloc(uint8_t * start, Tree & tree, int psLog);
  
  /**
   * Allocates memory or returns NULL on failure.
   */
  void * Alloc(size_t size);
  
  /**
   * Attempts to allocate memory of a certain size, aligned by a certain size.
   * @discussion The alignment must be a power of two.
   */
  void * Align(size_t size, size_t align);
  
  /**
   * Free a buffer by passing the pointer to its start.
   */
  void Free(void * buff);
  
  /**
   * Check if a pointer is contained within the boundary of the allocator's
   * buffer.
   */
  bool OwnsPointer(void * ptr) const;
  
  /**
   * Returns the internal tree used by the allocator.
   */
  const Tree & GetTree() const;
  
  /**
   * Returns the log base 2 of the base node size.
   */
  int GetPageSizeLog() const;
  
protected:
  uint8_t * start;
  uint64_t length;
  
  Tree & tree;
  int psLog;
  
  int DepthForSize(size_t size) const;
};

template <class T>
Malloc * Malloc::WrapRegion(uint8_t * start, size_t length, int psLog) {
  int sizeLog = Log2Floor(length);
  int maxDepth = sizeLog - psLog;
  if (maxDepth < 0) return NULL;
  
  size_t useLength = sizeof(T) + sizeof(Malloc) + T::MemorySize(maxDepth + 1);
  if (useLength > length) return NULL;
  
  T * tree = (T *)start;
  Malloc * result = (Malloc *)(start + sizeof(T));
  uint8_t * bmBuffer = start + sizeof(T) + sizeof(Malloc);
  
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
