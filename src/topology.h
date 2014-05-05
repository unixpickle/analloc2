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
    return *this;
  }
  
  uintptr_t GetStart() const {
    return start;
  }
  
  uintptr_t GetEnd() const {
    return start + size;
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
  
  Description(const Description & desc) {
    *this = desc;
  }
  
  Description & operator=(const Description & desc) {
    start = desc.start;
    depth = desc.depth;
    return *this;
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
  
  Description descriptions[maxCount];
  int descriptionCount;

  size_t alignment;
  size_t minAlignment;
  size_t pageSize;
  
  Region * regions;
  int regionCount;
  
  bool FindLargestDescription(Description & desc) {
    desc.depth = 0;
    for (int i = 0; i < regionCount; i++) {
      Region & reg = regions[i];
      Description result;
      if (!RegionLargestFree(reg, result)) continue;
      
      if (result.depth > desc.depth) {
        desc.depth = result.depth;
        desc.start = result.start;
      } 
    }
    return desc.depth != 0;
  }
  
  bool RegionLargestFree(Region & reg, Description & desc) {
    uintptr_t currentLoc = NextFreeAligned(reg, reg.GetStart());    
    desc.depth = 0;
    
    while (currentLoc < reg.GetEnd()) {
      uintptr_t nextFree;
      uintptr_t endBreak = NextBreak(reg, currentLoc, &nextFree);
      
      size_t size = (size_t)(endBreak - currentLoc);
      int depth = SizeDepth(size);
      if (depth > desc.depth) {
        desc.depth = depth;
        desc.start = currentLoc;
      }
      
      currentLoc = NextFreeAligned(reg, nextFree);
    }
    return desc.depth != 0;
  }
  
  uintptr_t NextFreeAligned(Region & reg, uintptr_t loc) {
    while (loc < reg.GetEnd()) {
      if (loc % alignment) {
        loc += alignment - (loc % alignment);
      }
      if (loc >= reg.GetEnd()) break;
      
      // check if there is a region which contains this location
      bool isContained = false;
      for (int i = 0; i < descriptionCount; i++) {
        Description & desc = descriptions[i];
        if (desc.start > loc) continue;
        if (desc.start + DepthSize(desc.depth) > loc) {
          isContained = true;
          loc = desc.start + DepthSize(desc.depth);
          break;
        }
      }
      if (!isContained) {
        return loc;
      }
    }
    return reg.GetEnd();
  }
  
  uintptr_t NextBreak(Region & reg, uintptr_t loc, uintptr_t * nextFree) {
    uintptr_t nextBreak = reg.GetStart() + reg.GetSize();
    if (nextFree) *nextFree = nextBreak;
    for (int i = 0; i < descriptionCount; i++) {
      Description & desc = descriptions[i];
      if (desc.start < loc) continue;
      if (desc.start >= nextBreak) continue;
      nextBreak = descriptions[i].start;
      if (nextFree) {
        *nextFree = nextBreak + DepthSize(desc.depth);
      }
    }
    return nextBreak;
  }
  
  size_t DepthSize(int depth) {
    if (!depth) return 0;
    return (pageSize << (depth - 1));
  }
  
  int SizeDepth(size_t size) {
    if (!size) return 0;
    
    for (int i = 1; i < 64; i++) {
      size_t aSize = pageSize << (i - 1);
      if (aSize > size) {
        return i - 1;
      } else if (aSize == size) {
        return i;
      }
    }
    
    return 0;
  }

public:
  
  AllocatorList(size_t _alignment, size_t _minAlignment,
                size_t _pageSize, Region * _regions,
                int _regionCount) {
    alignment = _alignment;
    minAlignment = _minAlignment;
    pageSize = _pageSize;
    regions = _regions;
    regionCount = _regionCount;
    descriptionCount = 0;
  }
  
  void GenerateDescriptions() {
    while (descriptionCount < maxCount) {
      Description desc;
      if (!FindLargestDescription(descriptions[descriptionCount])) {
        alignment >>= 1;
        if (alignment < minAlignment) break;
        continue;
      }
      descriptionCount++;
    }
  }
  
  size_t BitmapByteCount() {
    size_t res = 0;
    for (int i = 0; i < descriptionCount; i++) {
      res += TreeType::MemorySize(descriptions[i].depth);
    }
    return res;
  }
  
  void GenerateAllocators(uint8_t * buffStart) {
    for (int i = 0; i < descriptionCount; i++) {
      TreeType tree(descriptions[i].depth, buffStart);
      buffStart += TreeType::MemorySize(descriptions[i].depth);
      trees[i] = tree;
      
      Allocator<TreeType> al(&tree);
      allocators[i] = al;
    }
  }
  
  void ReserveRegion(const Region & reg) {
    // use more Vodo here and hack the planet
    (void)reg;
  }
  
  const Description * GetDescriptions() {
    return static_cast<const Description *>(descriptions);
  }
  
  int GetDescriptionCount() {
    return descriptionCount;
  }
  
  // TODO: here, add more methods for allocating regions of memory, aligning
  // them, freeing them, etc.
  
};

}

#endif // __ANALLOC2_TOPOLOGY_H__
