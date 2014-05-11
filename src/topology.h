#ifndef __ANALLOC2_TOPOLOGY_H__
#define __ANALLOC2_TOPOLOGY_H__

#include "allocator.h"
#include "utility.h"
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
template <int maxCount, class TreeType>
class AllocatorList {
protected:
  Allocator<TreeType> allocators[maxCount];
  TreeType trees[maxCount];
  
  Description descriptions[maxCount];
  int descriptionCount;

  size_t alignment;
  size_t minAlignment;
  size_t pageSize;
  size_t availableSpace;
  
  Region * regions;
  int regionCount;
  
  bool FindLargestDescription(Description & desc);
  bool RegionLargestFree(Region & reg, Description & desc);
  uintptr_t NextFreeAligned(Region & reg, uintptr_t loc);
  uintptr_t NextBreak(Region & reg, uintptr_t loc, uintptr_t * nextFree);
  size_t DepthSize(int depth);
  int SizeDepth(size_t size);
  Path FloorBasePath(Description & desc, uintptr_t byteIndex);
  size_t CountBaseNodes(Description & desc, uintptr_t start, uintptr_t end);

public:
  
  AllocatorList(size_t _alignment, size_t _minAlignment,
                size_t _pageSize, Region * _regions,
                int _regionCount);
  AllocatorList();
  
  /**
   * Setup structures for the next call, BitmapByteCount().
   */
  void GenerateDescriptions();
  
  /**
   * Returns the total number of bytes required to be allocated in the buffer
   * which you will pass to GenerateAllocators().
   */
  size_t BitmapByteCount();
  
  /**
   * You are almost done initialization! Now, take your chance to call
   * Reserve() if you need to.
   */
  void GenerateAllocators(uint8_t * buffStart);
  
  /**
   * Reserve a certain region of physical memory so that it can never be
   * returned. This is probably the region of memory which contains the bitmap
   * data and other structures that were allocated with a previous allocator.
   */
  void Reserve(const Region & reg);
  
  /**
   * Returns the array of descriptions.
   */
  const Description * GetDescriptions();
  
  /**
   * Returns the number of allocators and thus the number of descriptions.
   */
  int GetDescriptionCount();
  
  /**
   * Returns the array of allocator trees.
   */
  TreeType * GetTrees();
  
  /**
   * Returns the array of allocators themselves.
   */
  Allocator<TreeType> * GetAllocators();
  
  /**
   * Get a pointer for a path in a given allocator.
   */
  uintptr_t PointerForPath(int allocIndex, Path p);
  
  /**
   * Get a path and allocator for a given pointer.
   */
  bool PathForPointer(uintptr_t ptr, Path & path, int & i);

  /**
   * Allocate a path of a certain size.
   */
  bool AllocPath(size_t size, unsigned int alignLog, Path & p, int & i);

  /**
   * Allocate in the "bad" way, grabbing twice the needed size in order to get
   * an aligned address back.
   */
  bool BadAlloc(size_t size, size_t alignLog, Path & p, int & i);

  /**
   * This is the highest-level allocation method, and thus you will most likely
   * stick to this. This is the only allocation method which actually maintains
   * the available space counter.
   */
  bool AllocPointer(size_t size,
                    size_t align,
                    uintptr_t & out,
                    size_t * sizeOut);

  /**
   * High-level free method. Stick to this.
   */
  void FreePointer(uintptr_t ptr);
  
  /**
   * Returns the number of bytes that may be allocated.
   */
  size_t AvailableSpace();
 
};

/**********
 * Public *
 **********/

template <int mc, class T>
AllocatorList<mc, T>::AllocatorList(size_t _alignment, size_t _minAlignment,
                                    size_t _pageSize, Region * _regions,
                                    int _regionCount) {
  assert((1L << Log2Floor(_alignment)) == _alignment);
  assert((1L << Log2Floor(_pageSize)) == _pageSize);
  alignment = _alignment;
  minAlignment = _minAlignment;
  pageSize = _pageSize;
  regions = _regions;
  regionCount = _regionCount;
  descriptionCount = 0;
  availableSpace = 0;
}

template <int mc, class T>
AllocatorList<mc, T>::AllocatorList() {
  alignment = 0;
  minAlignment = 0;
  pageSize = 0;
  regions = NULL;
  regionCount = 0;
  descriptionCount = 0;
  availableSpace = 0;
}

template <int mc, class T>
void AllocatorList<mc, T>::GenerateDescriptions() {
  while (descriptionCount < mc) {
    Description desc;
    if (!FindLargestDescription(descriptions[descriptionCount])) {
      alignment >>= 1;
      if (alignment < minAlignment) break;
      continue;
    }
    descriptionCount++;
  }
}

template <int mc, class T>
size_t AllocatorList<mc, T>::BitmapByteCount() {
  size_t res = 0;
  for (int i = 0; i < descriptionCount; i++) {
    res += T::MemorySize(descriptions[i].depth);
  }
  return res;
}

template <int mc, class T>
void AllocatorList<mc, T>::GenerateAllocators(uint8_t * buffStart) {
  for (int i = 0; i < descriptionCount; i++) {
    T tree(descriptions[i].depth, buffStart);
    buffStart += T::MemorySize(descriptions[i].depth);
    trees[i] = tree;
    
    Allocator<T> al(&trees[i]);
    allocators[i] = al;
    
    availableSpace += pageSize << (descriptions[i].depth - 1);
  }
}

template <int mc, class T>
void AllocatorList<mc, T>::Reserve(const Region & reg) {
  size_t usedSize = 0;
  
  for (int i = 0; i < descriptionCount; i++) {
    Description & desc = descriptions[i];
    uintptr_t descEnd = desc.start + DepthSize(desc.depth);
    if (reg.GetStart() > descEnd) {
      continue;
    } else if (reg.GetEnd() <= desc.start) {
      continue;
    }
    
    uintptr_t chunkStart, chunkEnd;
    if (reg.GetStart() > desc.start) {
      chunkStart = reg.GetStart();
    } else {
      chunkStart = desc.start;
    }
    if (reg.GetEnd() < descEnd) {
      chunkEnd = reg.GetEnd();
    } else {
      chunkEnd = descEnd;
    }
    
    Path startPath = FloorBasePath(desc, chunkStart);
    size_t pathCount = CountBaseNodes(desc, chunkStart, chunkEnd);
    
    if (!pathCount) continue;
    
    Path p;
    bool result = allocators[i].Alloc(0, p);
    assert(result);
    
    size_t dataSize = 1L << (desc.depth - 1);
    allocators[i].Reserve(p, startPath, (uintptr_t)pathCount);
    usedSize += pageSize * pathCount;
  }
  
  assert(usedSize <= availableSpace);
  availableSpace -= usedSize;
}

template <int mc, class T>
const Description * AllocatorList<mc, T>::GetDescriptions() {
  return static_cast<Description * const>(descriptions);
}

template <int mc, class T>
int AllocatorList<mc, T>::GetDescriptionCount() {
  return descriptionCount;
}

template <int mc, class T>
T * AllocatorList<mc, T>::GetTrees() {
  return trees;
}

template <int mc, class T>
Allocator<T> * AllocatorList<mc, T>::GetAllocators() {
  return allocators;
}

template <int mc, class T>
uintptr_t AllocatorList<mc, T>::PointerForPath(int allocIndex, Path p) {
  Description & desc = descriptions[allocIndex];
  int depth = PathDepth(p);
  
  size_t eachSize = (pageSize << (desc.depth - depth - 1));
  return desc.start + eachSize * PathIndex(p);
}

template <int mc, class T>
bool AllocatorList<mc, T>::PathForPointer(uintptr_t ptr, Path & path,
                                          int & i) {
  for (i = 0; i < descriptionCount; i++) {
    Description & desc = descriptions[i];
    uintptr_t descEnd = desc.start + DepthSize(desc.depth);
    if (ptr < desc.start) continue;
    if (ptr >= descEnd) continue;
    
    // we found our descriptor
    uintptr_t baseIndex = (ptr - desc.start) / pageSize;
    if (!allocators[i].Find(baseIndex, path)) {
      return false;
    }
    return true;
  }
  return false;
}

template <int mc, class T>
bool AllocatorList<mc, T>::AllocPath(size_t size, unsigned int alignLog,
                                     Path & p, int & i) {
  size_t alignSize = (1L << alignLog);
  size_t grabSize = size > alignSize ? size : alignSize;
  int grabPower = Log2Ceil(grabSize) - Log2Ceil(pageSize);
  if (grabPower < 0) grabPower = 0;

  for (i = 0; i < descriptionCount; i++) {
    Description & desc = descriptions[i];
    uintptr_t start = desc.start;
    // make sure `start` is aligned to `align`
    if (start & (alignSize - 1)) continue;
    // make sure its big enough
    if (desc.depth <= grabPower) continue;
    // calculate the depth of the node we need
    int reqDepth = desc.depth - grabPower - 1;
    if (allocators[i].Alloc(reqDepth, p)) {
      return true;
    }
  }
  if (alignLog > 0) return BadAlloc(size, alignLog, p, i);
  return false;
}

template <int mc, class T>
bool AllocatorList<mc, T>::BadAlloc(size_t size, size_t alignLog,
                                    Path & p, int & i) {
  size_t alignSize = (1L << alignLog);
  size_t grabSize = size > alignSize ? size : alignSize;
  
  return AllocPath(grabSize * 2, 0, p, i);
}

template <int mc, class T>
bool AllocatorList<mc, T>::AllocPointer(size_t size,
                                        size_t align,
                                        uintptr_t & out,
                                        size_t * sizeOut) {
  int alignLog = Log2Floor(align);
  assert((1L << alignLog) == align);

  Path p;
  int i;
  if (!AllocPath(size, alignLog, p, i)) {
    return false;
  }
  
  out = PointerForPath(i, p);
  
  size_t usedSize = 0;
  int power = descriptions[i].depth - PathDepth(p) - 1;
  usedSize = pageSize << power;
  if (sizeOut) *sizeOut = usedSize;
  
  // make sure the actual returned address is aligned (it might not be if)
  // our call got forwarded to BadAlloc().
  if (out % align) {
    if (sizeOut) *sizeOut -= align - (out % align);
    out += align - (out % align);
  }
  
  availableSpace -= usedSize;
  
  return true;
}

template <int mc, class T>
void AllocatorList<mc, T>::FreePointer(uintptr_t ptr) {
  Path p;
  int i;
  if (!PathForPointer(ptr, p, i)) return;
  
  int depth = descriptions[i].depth;
  size_t dataSize = pageSize << (depth - 1 - PathDepth(p));
  availableSpace += dataSize;
  
  allocators[i].Free(p);
}

template <int mc, class T>
size_t AllocatorList<mc, T>::AvailableSpace() {
  return availableSpace;
}

/***********
 * Private *
 ***********/

template <int mc, class T>
bool AllocatorList<mc, T>::FindLargestDescription(Description & desc) {
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

template <int mc, class T>
bool AllocatorList<mc, T>::RegionLargestFree(Region & reg,
                                             Description & desc) {
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

template <int mc, class T>
uintptr_t AllocatorList<mc, T>::NextFreeAligned(Region & reg, uintptr_t loc) {
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

template <int mc, class T>
uintptr_t AllocatorList<mc, T>::NextBreak(Region & reg, uintptr_t loc,
                                          uintptr_t * nextFree) {
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

template <int mc, class T>
size_t AllocatorList<mc, T>::DepthSize(int depth) {
  if (!depth) return 0;
  return (pageSize << (depth - 1));
}

template <int mc, class T>
int AllocatorList<mc, T>::SizeDepth(size_t size) {
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

template <int mc, class T>
Path AllocatorList<mc, T>::FloorBasePath(Description & desc,
                                         uintptr_t byteIndex) {
  uintptr_t diff = byteIndex - desc.start;
  return (Path)(diff / pageSize);
}

template <int mc, class T>
size_t AllocatorList<mc, T>::CountBaseNodes(Description & desc,
                                            uintptr_t start,
                                            uintptr_t end) {
  Path pathStart = FloorBasePath(desc, start);
  uintptr_t realStart = desc.start + (pathStart * pageSize);
  uintptr_t diff = end - realStart;
  return (size_t)(diff / pageSize) + (diff % pageSize ? 1L : 0L);
}

}

#endif // __ANALLOC2_TOPOLOGY_H__
