#ifndef __ANALLOC2_DESC_HPP__
#define __ANALLOC2_DESC_HPP__

#include "../int.hpp"

namespace ANAlloc {

class Desc {
public:
  inline Desc() {
  }
  
  inline Desc(UInt _start, int _depth) : start(_start), depth(_depth) { 
  }
  
  inline Desc(const Desc & d) : start(d.start), depth(d.depth) {
  }
  
  inline Desc & operator=(const Desc & d) {
    start = d.start;
    depth = d.depth;
    return *this;
  }
  
  inline UInt GetStart() const {
    return start;
  }
  
  inline int GetDepth() const {
    return depth;
  }
  
private:
  UInt start;
  int depth;
};

}

#endif
