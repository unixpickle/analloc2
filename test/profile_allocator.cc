#include "microtime.hpp"
#include "../src/allocator.hpp"
#include "../src/btree.hpp"
#include "../src/bbtree.hpp"
#include <iostream>
#include <cassert>

using namespace ANAlloc;

template <class T>
uint64_t TimeLeafAllocation();

template <class T>
uint64_t TimeSplit();

template <class T>
uint64_t TimeFind();

int main() {
  std::cout << "Allocator<BBTree>::Split() ... " << std::flush
    << TimeSplit<BBTree>() << std::endl;
  std::cout << "Allocator<BTree>::Split() ... " << std::flush
    << TimeSplit<BTree>() << std::endl;
  
  std::cout << "Allocator<BBTree>::Find() ... " << std::flush
    << TimeFind<BBTree>() << std::endl;
  std::cout << "Allocator<BTree>::Find() ... " << std::flush
    << TimeFind<BTree>() << std::endl;
  
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

template <class T>
uint64_t TimeSplit() {
  static const int depth = 17;
  uint8_t * buf = new uint8_t[T::MemorySize(depth)];
  T tree(depth, buf);
  Allocator<T> alloc(&tree);
  uint64_t start = Microtime();
  
  Path p;
  bool res = alloc.Alloc(0, p);
  assert(res);
  assert(p == 0);
  
  for (int i = 0; i < depth - 1; i++) {
    for (uint64_t j = 0; j < (1UL << i); j++) {
      Path path = (1UL << i) - 1 + j;
      alloc.Split(path);
    }
  }
  
  uint64_t result = Microtime() - start;
  delete buf;
  return result;
}

template <class T>
uint64_t TimeFind() {
  static const int depth = 17; // must be more than 12
  uint8_t * buf = new uint8_t[T::MemorySize(depth)];
  T tree(depth, buf);
  Allocator<T> alloc(&tree);
  
  // allocate the first node and time it
  uint64_t sum = 0;
  Path p;
  bool res = alloc.Alloc(0, p);
  assert(res);
  assert(p == 0);
  
  uint64_t start = Microtime();
  for (int i = 0; i < 0x10000; i++) {
    bool res = alloc.Find(i, p);
    assert(res);
    assert(p == 0);
  }
  sum += Microtime() - start;
  
  // allocate the leftmost node
  alloc.Reserve(0, 0, 1);
  
  for (int i = 0; i < 0x10000; i++) {
    alloc.Find(0, p);
  }
  
  delete buf;
  return sum;
}
