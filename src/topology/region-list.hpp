#ifndef __ANALLOC2_REGION_LIST_HPP__
#define __ANALLOC2_REGION_LIST_HPP__

#include "region.hpp"

namespace ANAlloc {

class RegionList {
public:
  virtual const Region & operator[](int idx) const = 0;
  virtual int GetCount() const = 0;
};

}

#endif
