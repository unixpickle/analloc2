#ifndef __ANALLOC2_FIXED_DESC_LIST_HPP__
#define __ANALLOC2_FIXED_DESC_LIST_HPP__

#include "region-list.hpp"
#include <cassert>

namespace ANAlloc {

template <size_t capacity>
class FixedDescList : public DescList {
public:
  // overloaded methods
  virtual Desc & operator[](int idx) {
    return descs[idx];
  }
  
  virtual int GetCount() {
    return count;
  }
  
  virtual bool Push(const Desc & reg) {
    if (count >= capacity) return false;
    descs[count++] = reg;
    return true;
  }
  
private:
  Desc descs[capacity];
  int count;
};

}

#endif
