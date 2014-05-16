#ifndef __ANALLOC2_TOPOLOGY_H__
#define __ANALLOC2_TOPOLOGY_H__

#include "../allocator.hpp"
#include "../utility.hpp"
#include "region.hpp"

namespace ANAlloc {

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
  int SizeDepth(size_t size);
  Path FloorBasePath(Description & desc, uintptr_t byteIndex);
  size_t CountBaseNodes(Description & desc, uintptr_t start, uintptr_t end);
  void InsertDescription(const Description & desc, bool sorted);

public:
  AllocatorList(size_t _alignment, size_t _minAlignment,
                size_t _pageSize, Region * _regions,
                int _regionCount);
  AllocatorList();
  
  void SetInformation(size_t _alignment, size_t _minAlignment,
                      size_t _pageSize, Region * _regions,
                      int _regionCount);
  
  /**
   * Setup structures for the next call, BitmapByteCount().
   */
  void GenerateDescriptions(bool sorted = false);
  
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

}

#include "topology-public.hpp"
#include "topology-protected.hpp"

#endif // __ANALLOC2_TOPOLOGY_H__

