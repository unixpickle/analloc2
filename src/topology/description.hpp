#ifndef __ANALLOC2_DESCRIPTION_H__
#define __ANALLOC2_DESCRIPTION_H__

#include <cstdint>
#include <cstddef>
#include <cassert>

namespace ANAlloc {

/**
* Represents the space to be used by an allocator.
*/
class Description {
protected:
  uintptr_t start = 0;
  int depth = 0;
  size_t pageSize = 0;

public:
  Description();
  Description(size_t pageSize);
  Description(size_t pageSize, uintptr_t start, int depth);
  Description(const Description & desc);
  
  Description & operator=(const Description & desc);
  int GetDepth() const;
  size_t GetSize() const;
  uintptr_t GetStart() const;
  uintptr_t GetEnd() const;
  bool Contains(uintptr_t ptr) const;
  
  void SetDepth(int depth);
  void SetStart(uintptr_t start);
  
};

}

#endif
