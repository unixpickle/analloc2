#ifndef __ANALLOC2_FIXED_DESC_LIST_HPP__
#define __ANALLOC2_FIXED_DESC_LIST_HPP__

#include "region-list.hpp"
#include <cassert>

namespace ANAlloc {

template <int capacity>
class FixedDescList : public DescList {
public:
  // overloaded methods
  virtual Desc & operator[](int idx) {
    return descs[idx];
  }
  
  virtual const Desc & operator[](int idx) const {
    return descs[idx];
  }
  
  virtual int GetCount() const {
    return count;
  }
  
  virtual bool Push(const Desc & reg) {
    if (count >= capacity) return false;
    descs[count++] = reg;
    return true;
  }
  
  virtual void Empty() {
    count = 0;
  }
  
private:
  Desc descs[capacity];
  int count = 0;
};

}

#endif
