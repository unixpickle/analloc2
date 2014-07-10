#ifndef __ANALLOC2_MUTABLE_CLUSTER_HPP__
#define __ANALLOC2_MUTABLE_CLUSTER_HPP__

#include "cluster.hpp"

namespace ANAlloc {

class MutableCluster : public Cluster {
public:
  virtual bool Push(UInt start, Tree & tree, int psLog) = 0;
};

}

#endif
