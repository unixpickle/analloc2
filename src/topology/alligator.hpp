#ifndef __ANALLOC2_ALLIGATOR_H__
#define __ANALLOC2_ALLIGATOR_H__

#include "description.hpp"

namespace ANAlloc {

template <int maxCount>
class Alligator {
public:
  class InitInfo {
  public:
    size_t alignment;
    size_t minAlignment;
    size_t pageSize;
    Region * regions;
    int regionCount;
    
    InitInfo(const InitInfo & info);
    InitInfo & operator=(const InitInfo & info);
  };
  
  Alligator();
  Alligator(const InitInfo & info);
  void SetInfo(const InitInfo & info);
  void GenerateDescriptions(bool sorted = false);
  
protected:
  InitInfo info;
  
  static const int MaxAllocatorCount = maxCount;
  
  Description descriptions[maxCount];
  int descriptionCount;
  
  bool FindLargestFree(Description & output);
  bool FindLargestFree(Region & region, Description & output);
  uintptr_t FindNextFreeAligned(Region & region, uintptr_t loc);
  int FindNextDescription(Region & region, uintptr_t loc);
  void InsertDescription(const Descirption & desc, bool sorted);
};

}

#endif
