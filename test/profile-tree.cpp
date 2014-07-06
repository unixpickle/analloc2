#include "../src/bbtree.hpp"
#include "../src/btree.hpp"
#include <sys/time.h>
#include <cstdint>
#include <iostream>

using namespace ANAlloc;

uint64_t Microtime();

template <class T>
uint64_t TimeBranchAllocation();

template <class T>
uint64_t TimeOrderedAllocation();

template <class T>
uint64_t TimeBaseAllocation();

void AllocAllButMiddle(Tree & tree, Path p);

int main() {
  std::cout << "BBTree::[Set/Get]Type()... " << std::flush
    << TimeBranchAllocation<BBTree>() << std::endl;
  std::cout << "BBTree::FindFree() [ordered]... " << std::flush
    << TimeOrderedAllocation<BBTree>() << std::endl;
  std::cout << "BBTree::FindFree() [single-page]... " << std::flush
    << TimeBaseAllocation<BBTree>() << std::endl;
  std::cout << "BTree::[Set/Get]Type()... " << std::flush
    << TimeBranchAllocation<BTree>() << std::endl;
  std::cout << "BTree::FindFree() [ordered]... " << std::flush
    << TimeOrderedAllocation<BTree>() << std::endl;
  std::cout << "BTree::FindFree() [single-page]... " << std::flush
    << TimeBaseAllocation<BTree>() << std::endl;
  return 0;
}

uint64_t Microtime() {
  timeval time;
  gettimeofday(&time, NULL);
  return (uint64_t)time.tv_sec * 1000000L + (uint64_t)time.tv_usec;
}

template <class T>
uint64_t TimeBranchAllocation() {
  uint8_t * buf = new uint8_t[T::MemorySize(28)];
  T tree(28, buf);
  uint64_t start = Microtime();
  
  for (int i = 0; i < 0x1000; i++) {
    Path p;
    for (int depth = 0; depth < 27; depth++) {
      tree.SetType(p, NodeTypeContainer);
      tree.SetType(p.Left(), NodeTypeData);
      if (depth == 26) {
        tree.SetType(p.Right(), NodeTypeData);
      }
      p = p.Right();
    }
    for (int depth = 27; depth > 0; depth--) {
      tree.SetType(p.Sibling(), NodeTypeFree);
      tree.SetType(p, NodeTypeFree);
      p = p.Parent();
    }
    tree.SetType(p, NodeTypeFree);
    assert(p == Path::RootPath());
  }
  
  uint64_t result = Microtime() - start;
  delete buf;
  return result;
}

template <class T>
uint64_t TimeOrderedAllocation() {
  uint8_t * buf = new uint8_t[T::MemorySize(28)];
  T tree(28, buf);
  uint64_t start = Microtime();
  
  for (int i = 0; i < 0x1000; i++) {
    for (int depth = 1; depth < 28; depth++) {
      Path p;
      tree.FindFree(depth, p);
      assert(p.GetDepth() == depth - 1);
      assert(tree.GetType(p) == NodeTypeFree);
      tree.SetType(p, NodeTypeContainer);
      tree.SetType(p.Left(), NodeTypeData);
      tree.SetType(p.Right(), NodeTypeFree);
    }
    
    tree.FreeAll();
  }
  
  uint64_t result = Microtime() - start;
  delete buf;
  return result;
}

template <class T>
uint64_t TimeBaseAllocation() {
  uint8_t * buf = new uint8_t[T::MemorySize(12)];
  T tree(12, buf);
  AllocAllButMiddle(tree, Path::RootPath());
  
  uint64_t start = Microtime();
  
  for (int i = 0; i < 0x1000; i++) {
    Path p;
    bool res = tree.FindFree(tree.GetDepth() - 1, p);
    assert(res);
    tree.SetType(p, NodeTypeData);
    
    Path nonP;
    res = tree.FindFree(tree.GetDepth() - 1, nonP);
    assert(!res);
    
    assert(p.GetDepth() == tree.GetDepth() - 1);
    assert(p.GetIndex() == (1 << (tree.GetDepth() - 1)) / 2);
    tree.SetType(p, NodeTypeFree);
  }
  
  uint64_t result = Microtime() - start;
  delete buf;
  return result;
}

void AllocAllButMiddle(Tree & tree, Path p) {
  if (p.GetDepth() + 1 == tree.GetDepth()) {
    uint64_t rowCount = Path::DepthCount(tree.GetDepth() - 1);
    if (p.GetIndex() == rowCount / 2) {
      tree.SetType(p, NodeTypeFree);
    } else {
      tree.SetType(p, NodeTypeData);
    }
  } else {
    tree.SetType(p, NodeTypeContainer);
    AllocAllButMiddle(tree, p.Right());
    AllocAllButMiddle(tree, p.Left());
  }
}
