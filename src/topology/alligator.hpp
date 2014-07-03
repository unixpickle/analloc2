#ifndef __ANALLOC2_ALLIGATOR_H__
#define __ANALLOC2_ALLIGATOR_H__

#include "description.hpp"
#include "region.hpp"

namespace ANAlloc {

template <int maxCount>
class Alligator {
public:
  class InitInfo {
  public:
    size_t alignment = 0;
    size_t minAlignment = 0;
    size_t pageSize = 0;
    Region * regions = 0;
    int regionCount = 0;
    
    InitInfo();
    InitInfo(size_t, size_t, size_t, Region *, int);
    InitInfo(const InitInfo & info);
    InitInfo & operator=(const InitInfo & info);
  };
  
  Alligator();
  Alligator(const InitInfo & info);
  void SetInfo(const InitInfo & info);
  void GenerateDescriptions(bool sorted = false);
  
protected:
  InitInfo info;
  
  static const int MaxDescriptionCount = maxCount;
  
  Description descriptions[maxCount];
  int descriptionCount = 0;
  
  bool FindLargestFree(Description & output);
  bool FindLargestFree(Region & region, Description & output);
  uintptr_t FindNextFreeAligned(Region & region, uintptr_t loc);
  int FindNextDescription(Region & region, uintptr_t loc);
  void InsertDescription(const Description & desc, bool sorted);
};

}

#include "alligator-protected.hpp"
#include "alligator-public.hpp"

#endif
