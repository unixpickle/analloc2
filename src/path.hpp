#ifndef __ANALLOC2_PATH_HPP__
#define __ANALLOC2_PATH_HPP__

#include <cstdint>
#include <cassert>

namespace ANAlloc {

class Path {
public:
  static Path RootPath() {
    return Path(0, 0);
  }
  
  inline static uint64_t DepthCount(int depth) {
    return 1UL << depth;
  }
  
  inline Path(int _depth, uint64_t _index) : depth(_depth), index(_index) {
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
  
  inline uint64_t GetIndex() const {
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
  
  inline uint64_t TreeIndex() const {
    return (1UL << depth) - 1UL + index;
  }
  
private:
  int depth;
  uint64_t index;
};

}

#endif
