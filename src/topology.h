#ifndef __ANALLOC2_TOPOLOGY_H__
#define __ANALLOC2_TOPOLOGY_H__

#include "allocator.h"

namespace ANAlloc {

/**
 * Represents an abstract region of memory.
 */
class Region {
private:
  uintptr_t start;
  size_t size;
  
public:
  Region(uintptr_t _start, size_t _size) {
    start = _start;
    size = _size;
  }
  
  Region() : start(0), size(0) {}
  
  Region(const Region & reg) {
    *this = reg;
  }
  
  Region & operator=(const Region & reg) {
    start = reg.start;
    size = reg.size;
  }
  
  uintptr_t GetStart() const {
    return start;
  }
  
  size_t GetSize() const {
    return size;
  }
   
};

/**
 * Represents the space to be used by an allocator.
 */
struct Description {
  uintptr_t start;
  int depth;
  
  Description() : start(0), depth(0) {}
  
  Description & Description(const Description & desc) {
    *this = desc;
  }
  
  Description & operator=(const Description & desc) {
    start = desc.start;
    depth = desc.depth;
  }
};

/**
 * This template class allows you to map out the topology of a memory space and
 * create a fitting amount of allocators.
 */
template <size_t maxCount, class TreeType>
class AllocatorList {
protected:
  Allocator<TreeType> allocators[maxCount];
  TreeType trees[maxCount];
  int allocatorCount;
  
  Description descriptions[maxCount];
  int descriptionCount;

  size_t alignment;
  size_t pageSize;

public:
  
  AllocatorList(size_t _alignment, size_t _pageSize) {
    alignment = _alignment;
    pageSize = _pageSize;
  }
  
  void GenerateDescriptions() {
    // use some VODO magic here
  }
  
  void GenerateAllocators() {
    // this isn't too hard, actually, but still vodooooooooooooo
  }
  
  void ReserveRegion(const Region & reg) {
    // use more Vodo here and hack the planet
    (void)reg;
  }
  
  // TODO: here, add more methods for allocating regions of memory, aligning
  // them, freeing them, etc.
  
};

}

#endif // __ANALLOC2_TOPOLOGY_H__
