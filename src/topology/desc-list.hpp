#ifndef __ANALLOC2_DESC_LIST_HPP__
#define __ANALLOC2_DESC_LIST_HPP__

#include "desc.hpp"

namespace ANAlloc {

class DescList {
public:
  virtual Desc & operator[](int idx) = 0;
  virtual const Desc & operator[](int idx) const = 0;
  virtual int GetCount() const = 0;
  virtual bool Push(const Desc & d) = 0;
};

}

#endif
