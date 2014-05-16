#ifndef __ANALLOC2_TOPOLOGY_H__
#define __ANALLOC2_TOPOLOGY_H__

#include "../allocator.hpp"
#include "../utility.hpp"
#include "region.hpp"
#include "description.hpp"
#include "alligator.hpp"

namespace ANAlloc {

/**
 * This template class allows you to map out the topology of a memory space and
 * create a fitting amount of allocators.
 */
template <int maxCount, class TreeType>
class AllocatorList : Alligator<maxCount> {
public:
  typedef typename Alligator<maxCount> AlligatorType;
  typedef AlligatorType super;
  typedef typename T TreeType;
  
  AllocatorList(const AlligatorType::InitInfo & info);
  AllocatorList();
  
  size_t BitmapByteCount();
  void GenerateAllocators(uint8_t * buffStart);
  void Reserve(uintptr_t endReserved);
  
  const Description * GetDescriptions();
  int GetDescriptionCount();
  TreeType * GetTrees();
  Allocator<TreeType> * GetAllocators();
  
  size_t GetAvailableSpace();
 
protected:
  Allocator<TreeType> allocators[maxCount];
  TreeType trees[maxCount];
  
  size_t availableSpace;
};

}

#include "topology-public.hpp"
#include "topology-protected.hpp"

#endif // __ANALLOC2_TOPOLOGY_H__

