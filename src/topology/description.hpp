#ifndef __ANALLOC2_DESCRIPTION_HPP__
#define __ANALLOC2_DESCRIPTION_HPP__

#include "../int-type.hpp"

namespace ANAlloc {

/**
* Represents the space to be used by an allocator.
*/
class Description {
public:
  inline Description() {
  }
  
  inline Description(Integer _start, Integer _pageSize, int _depth)
    : start(_start), pageSize(_pageSize), depth(_depth) {
  }
  
  inline Description(const Description & desc)
    : start(desc.start), pageSize(desc.pageSize), depth(desc.depth) {    
  }
  
  inline Description & operator=(const Description & desc) {
    start = desc.start;
    pageSize = desc.pageSize;
    depth = desc.depth;
  }
  
  inline int GetDepth() const {
    return depth;
  }
  
  inline Integer GetSize() const {
    return pageSize << (depth - 1);
  }
  
  inline Integer GetPageSize() const {
    return pageSize;
  }
  
  inline Integer GetStart() const {
    return start;
  }
  
  inline Integer GetEnd() const {
    return GetStart() + GetSize();
  }
  
  bool Contains(Integer ptr) const {
    if (ptr < start) return false;
    else if (ptr >= GetEnd()) return false;
    return true;
  }

protected:
  Integer start = 0;
  Integer pageSize = 0;
  int depth = 0;
};

}

#endif
