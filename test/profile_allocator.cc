#include "microtime.hpp"
#include "../src/allocator.hpp"
#include "../src/btree.hpp"
#include "../src/bbtree.hpp"
#include <iostream>
#include <cassert>

using namespace ANAlloc;

template <class T>
uint64_t TimeLeafAllocation();

int main() {
  std::cout << "Allocator<BBTree>::[Alloc/Free]() ... " << std::flush
    << TimeLeafAllocation<BBTree>() << std::endl;
  std::cout << "Allocator<BTree>::[Alloc/Free]() ... " << std::flush
    << TimeLeafAllocation<BTree>() << std::endl;
  return 0;
}

template <class T>
uint64_t TimeLeafAllocation() {
  static const int depth = 15;
  
  uint8_t * buf = new uint8_t[T::MemorySize(depth)];
  T tree(depth, buf);
  Allocator<T> alloc(&tree);
  uint64_t start = Microtime();
  
  for (int i = 0; i < (1 << (depth - 1)); i++) {
    Path p;
    bool res = alloc.Alloc(depth - 1, p);
    assert(res);
    assert(PathDepth(p) == depth - 1);
  }
  Path noP;
  assert(!alloc.Alloc(depth - 1, noP));
  for (uint64_t i = 0; i < (1 << (depth - 1)); i++) {
    Path thePath = (1 << (depth - 1)) - 1 + i;
    alloc.Free(thePath);
  }
  
  assert(tree.GetType(0) == T::NodeTypeFree);
  
  uint64_t result = Microtime() - start;
  delete buf;
  return result;
}
