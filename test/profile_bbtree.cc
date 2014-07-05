#include "../src/bbtree.hpp"
#include <sys/time.h>
#include <cstdint>
#include <iostream>

using namespace ANAlloc;

uint64_t Microtime();
uint64_t TimeBranchAllocation();

int main() {
  std::cout << "\"branch\" allocation - " << TimeBranchAllocation()
    << std::endl;
  return 0;
}

uint64_t Microtime() {
  timeval time;
  gettimeofday(&time, NULL);
  return (uint64_t)time.tv_sec * 1000000L + (uint64_t)time.tv_usec;
}

uint64_t TimeBranchAllocation() {
  uint8_t * buf = new uint8_t[BBTree::MemorySize(28)];
  BBTree * tree = new BBTree(28, buf);
  uint64_t start = Microtime();
  
  for (int i = 0; i < 0x1000; i++) {
    Path p = 0;
    for (int depth = 0; depth < 27; depth++) {
      tree->SetType(p, Tree::NodeTypeContainer);
      tree->SetType(PathLeft(p), Tree::NodeTypeData);
      if (depth == 26) {
        tree->SetType(PathRight(p), Tree::NodeTypeData);
      }
      p = PathRight(p);
    }
    for (int depth = 27; depth >= 0; depth--) {
      tree->SetType(PathSibling(p), Tree::NodeTypeFree);
      tree->SetType(p, Tree::NodeTypeFree);
      p = PathParent(p);
    }
    tree->SetType(p, Tree::NodeTypeFree);
    assert(p == 0);
  }
  
  uint64_t result = Microtime() - start;
  delete tree;
  delete buf;
  return result;
}
