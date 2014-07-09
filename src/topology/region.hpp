#ifndef __ANALLOC2_REGION_HPP__
#define __ANALLOC2_REGION_HPP__

#include "../int-type.hpp"

namespace ANAlloc {

/**
 * Represents an abstract region of memory.
 */
class Region {
public:
  inline Region() : start(0), size(0), {
  }
  
  inline Region(Integer _start, Integer _size) : start(_start), size(_size) {
  }
  
  Region(const Region & reg) : start(reg.start), size(reg.size) {
  }
  
  Region & operator=(const Region & reg) {
    start = reg.start;
    size = reg.size;
    return *this;
  }
  
  inline Integer GetStart() const {
    return start;
  }
  
  inline Integer GetEnd() const {
    return start + size;
  }
  
  inline Integer GetSize() const {
    return size;
  }
  
  bool Contains(Integer ptr) const {
    if (ptr < start) return false;
    if (ptr >= End()) return false;
    return true;
  }

private:
  Integer start;
  Integer size;
};

}

#endif
