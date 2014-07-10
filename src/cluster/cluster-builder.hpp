#ifndef __ANALLOC2_CLUSTER_BUILDER_HPP__
#define __ANALLOC2_CLUSTER_BUILDER_HPP__

#include "mutable-cluster.hpp"
#include "../topology/desc-list.hpp"

namespace ANAlloc {

/**
 * Builds a cluster of allocators from a DescList. You use a ClusterBuilder in
 * two steps: first, you ask it how much memory it needs, and then you pass it
 * memory of that length which it uses to generate the cluster. The Allocator
 * objects in the cluster will be unused, so you will be able to call Reserve()
 * if needed.
 */
template <class T>
class ClusterBuilder {
public:
  ClusterBuilder(const DescList & _descs, MutableCluster & _cluster,
                 int _psLog)
    : descs(_descs), cluster(_cluster), psLog(_psLog) {
  }
  
  UInt RequiredSpace() {
    UInt bitmapSize = 0;
    for (int i = 0; i < descs.GetCount(); i++) {
      bitmapSize += T::MemorySize(descs[i].GetDepth());
    }
    return bitmapSize + sizeof(T) * descs.GetCount();
  }
  
  void CreateAllocators(uint8_t * buffer) {
    uint8_t * bitmapStart = buffer + sizeof(T) * descs.GetCount();
    for (int i = 0; i < descs.GetCount(); i++) {
      int depth = descs[i].GetDepth();
      UInt start = descs[i].GetStart();
      
      T * tree = GetTreePointer(buffer, i);
      new(tree) T(depth, bitmapStart);
      bitmapStart += T::MemorySize(depth);
      
      cluster.Push(start, *tree, psLog);
    }
  }
  
protected:
  T * GetTreePointer(uint8_t * buffer, int i) {
    return (T *)(buffer + sizeof(T) * i);
  }
    
private:
  const DescList & descs;
  MutableCluster & cluster;
  int psLog;
};

}

#endif
