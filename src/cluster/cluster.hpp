#ifndef __ANALLOC2_CLUSTER_HPP__
#define __ANALLOC2_CLUSTER_HPP__

#include "../allocator.hpp"

namespace ANAlloc {

class Cluster {
public:
  virtual int GetCount() const = 0;
  virtual const Allocator & operator[](int idx) const = 0;
  virtual Allocator & operator[](int idx) = 0;
  
  virtual void Reserve(UInt start, UInt length);
  
  virtual UInt GetTotalSize() const;
  virtual UInt GetFreeSize() const;
  
  virtual bool Alloc(UInt size, UInt & addrOut);
  virtual bool Align(UInt size, UInt align, UInt & addrOut);
  virtual void Free(UInt addr);
};

}

#endif
