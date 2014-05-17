#ifndef __ANALLOC2_TOPOLOGY_H__
#define __ANALLOC2_TOPOLOGY_H__

#include "../allocator.hpp"
#include "../utility.hpp"
#include "alligator.hpp"

namespace ANAlloc {

/**
 * This template class allows you to map out the topology of a memory space and
 * create a fitting amount of allocators.
 */
template <int maxCount, class TreeType>
class AllocatorList : public Alligator<maxCount> {
public:
  typedef Alligator<maxCount> AlligatorType;
  typedef AlligatorType super;
  
  AllocatorList(const typename AlligatorType::InitInfo & info);
  AllocatorList(size_t alignment, size_t minAlignment, size_t pageSize,
                Region * regions, int regionCount);
  AllocatorList();
  
  size_t BitmapByteCount(); // call GenerateDescriptions() first!
  void GenerateAllocators(uint8_t * buffStart);
  void Reserve(uintptr_t endReserved);
  
  size_t GetPageSize();
  
  Description * GetDescriptions();
  int GetDescriptionCount();
  TreeType * GetTrees();
  Allocator<TreeType> * GetAllocators();
  
  size_t GetAvailableSpace();
  
  uintptr_t PointerForPath(int allocIndex, Path p);
  bool PathForPointer(uintptr_t ptr, Path & path, int & i);
  
  void FreePointer(uintptr_t ptr);
  
  // allocation method which prefers to be non-sorted
  
  bool AllocPointer(size_t size, size_t align,
                    uintptr_t & out, size_t * sizeOut);
  
  // allocation methods which require sorting
  
  bool AllocAscending(size_t size, size_t align,
                      uintptr_t & out, size_t * sizeOut);
  bool AllocAscending(size_t size, size_t align,
                      uintptr_t & out, size_t * sizeOut,
                      uintptr_t maximum);
  bool AllocDescending(size_t size, size_t align,
                       uintptr_t & out, size_t * sizeOut);
  bool AllocDescending(size_t size, size_t align,
                       uintptr_t & out, size_t * sizeOut,
                       uintptr_t minimum);
 
protected:
  Allocator<TreeType> allocators[maxCount];
  TreeType trees[maxCount];
  
  size_t availableSpace;
  bool wasSorted;
  
  bool AllocPath(size_t size, size_t align, Path & p, int & i,
                 int direction, uintptr_t boundary);
  bool AllocGeneral(size_t size, size_t align, uintptr_t & out,
                    size_t * sizeOut, int direction, uintptr_t boundary);
};

}

#include "allocator-list-public.hpp"
#include "allocator-list-protected.hpp"

#endif // __ANALLOC2_TOPOLOGY_H__

