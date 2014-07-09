#ifndef __ANALLOC2_PATH_HPP__
#define __ANALLOC2_PATH_HPP__

#include "../int.hpp"
#include <cassert>

namespace ANAlloc {

class Path {
public:
  inline static Path Root() {
    return Path(0, 0);
  }
  
  inline static Path FromTreeIndex(UInt idx) {
    int depth;
    for (depth = 63; depth >= 0; depth--) {
      if ((idx + 1) & (1L << depth)) break;
    }
    if (!depth) return Path(0, 0);
    return Path(depth, idx - (DepthCount(depth) - 1));
  }
  
  inline static UInt DepthCount(int depth) {
    return 1UL << depth;
  }
  
  inline Path() : depth(0), index(0) {
  }
  
  inline Path(int _depth, UInt _index) : depth(_depth), index(_index) {
  }
  
  inline Path(const Path & p) : depth(p.depth), index(p.index) {
  }
  
  inline Path & operator=(const Path & p) {
    depth = p.depth;
    index = p.index;
    return *this;
  }
  
  inline bool operator==(const Path & p) {
    return depth == p.depth && index == p.index;
  }
  
  inline bool operator!=(const Path & p) {
    return !(*this == p);
  }
  
  inline int GetDepth() const {
    return depth;
  }
  
  inline UInt GetIndex() const {
    return index;
  }
  
  inline Path Left() const {
    return Path(depth + 1, index * 2);
  }
  
  inline Path Right() const {
    return Path(depth + 1, index * 2 + 1);
  }
  
  inline Path Parent() const {
    assert(depth != 0);
    return Path(depth - 1, index >> 1);
  }
  
  inline Path Sibling() const {
    assert(depth != 0);
    if (index % 2) {
      return Path(depth, index - 1);
    } else {
      return Path(depth, index + 1);
    }
  }
  
  inline UInt TreeIndex() const {
    return (1UL << depth) - 1UL + index;
  }
  
private:
  int depth;
  UInt index;
};

}

#endif
