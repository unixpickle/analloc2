#ifndef __ANALLOC2_MALLOC_HPP__
#define __ANALLOC2_MALLOC_HPP__

#include "allocator.hpp"
#include <new>

namespace ANAlloc {

/**
 * This lightweight template wraps the tree of your choice to provide a simple
 * linear address allocator.
 */
class Malloc : protected Allocator {  
public:
  using Allocator::GetTree;
  using Allocator::GetPageSizeLog;
  typedef Allocator super;
  
  template <class T>
  static Malloc * WrapRegion(uint8_t * start, size_t length, int psLog,
                             size_t initUsed = 0);
  
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
};

template <class T>
Malloc * Malloc::WrapRegion(uint8_t * start, size_t length, int psLog,
                            size_t initUsed) {
  if (initUsed & 0xf) {
    initUsed += 0x10 - (initUsed & 0xf);
  }
  int sizeLog = Log2Floor(length);
  int maxDepth = sizeLog - psLog;
  if (maxDepth < 0) return NULL;
  
  size_t useLength = initUsed + sizeof(T) + sizeof(Malloc)
    + T::MemorySize(maxDepth + 1);
  if (useLength > length) return NULL;
  
  T * tree = (T *)(start + initUsed);
  Malloc * result = (Malloc *)(start + initUsed + sizeof(T));
  uint8_t * bmBuffer = start + initUsed + sizeof(T) + sizeof(Malloc);
  
  // create the tree
  new(tree) T(maxDepth + 1, bmBuffer);
  
  // carve out the beginning of the buffer
  UInt carvePageCount = useLength >> psLog;
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
