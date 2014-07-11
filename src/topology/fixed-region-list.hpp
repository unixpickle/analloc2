#ifndef __ANALLOC2_FIXED_REGION_LIST_HPP__
#define __ANALLOC2_FIXED_REGION_LIST_HPP__

#include "region-list.hpp"

namespace ANAlloc {

template <int capacity>
class FixedRegionList : public RegionList {
public:
  // overloaded methods
  virtual const Region & operator[](int idx) const {
    return regions[idx];
  }
  
  virtual int GetCount() const {
    return count;
  }
  
  // specific methods
  virtual Region & operator[](int idx) {
    return regions[idx];
  }
  
  virtual bool Push(const Region & reg) {
    if (count == capacity) return false;
    regions[count++] = reg;
    return true;
  }
  
  virtual bool Insert(const Region & reg, int idx) {
    if (count == capacity) return false;
    for (int i = idx; i < count; ++i) {
      regions[i + 1] = regions[i];
    }
    regions[idx] = reg;
    ++count;
  }
  
private:
  Region regions[capacity];
  int count = 0;
};

}

#endif
