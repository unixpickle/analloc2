#ifndef __ANALLOC2_REGION_HPP__
#define __ANALLOC2_REGION_HPP__

#include "../int.hpp"

namespace ANAlloc {

class Region {
public:
  inline Region() {
  }
  
  inline Region(UInt _start, UInt _size) : start(_start), size(_size) {
  }
  
  inline Region(const Region & reg) : start(reg.start), size(reg.size) {
  }
  
  inline Region & operator=(const Region & reg) {
    start = reg.start;
    size = reg.size;
    return *this;
  }
  
  inline bool Contains(UInt ptr) const {
    return ptr >= start && ptr < start + size;
  }
  
  inline UInt GetStart() const {
    return start;
  }
  
  inline UInt GetSize() const {
    return size;
  }
  
  inline UInt GetEnd() const {
    return GetStart() + GetSize();
  }
  
private:
  UInt start;
  UInt size;
};

}

#endif
