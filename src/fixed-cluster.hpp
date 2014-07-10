#ifndef __ANALLOC2_FIXED_CLUSTER_HPP__
#define __ANALLOC2_FIXED_CLUSTER_HPP__

#include "cluster.hpp"
#include <new>

namespace ANAlloc {

template <int capacity>
class FixedCluster : public Cluster {
public:
  virtual int GetCount() const {
    return count;
  }
  
  virtual const Allocator & operator[](int idx) const {
    return *((const Allocator *)&allocators[idx * sizeof(Allocator)]);
  }
  
  virtual Allocator & operator[](int idx) {
    return *((Allocator *)&allocators[idx * sizeof(Allocator)]);
  }
  
  virtual bool Push(UInt start, Tree & tree, int psLog) {
    if (count == capacity) return false;
    new(&allocators[sizeof(Allocator) * count]) Allocator(start, tree, psLog);
    ++count;
    return true;
  }
  
private:
  uint8_t allocators[capacity * sizeof(Allocator)] __attribute__((aligned(8)));
  int count = 0;
};

}

#endif
