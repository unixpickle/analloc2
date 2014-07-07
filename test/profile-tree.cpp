#include "../src/tree/bbtree.hpp"
#include "../src/tree/btree.hpp"
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

template <class T>
uint64_t TimeLeafAllocation();

template <class T>
uint64_t TimeFindByShadow();

template <class T>
uint64_t TimeLooseAlignment();

template <class T>
uint64_t TimeTightAlignment();

void AllocAllButMiddle(Tree & tree, Path p);

int main() {
  std::cout << "BBTree::[Set/Get]Type()... " << std::flush
    << TimeBranchAllocation<BBTree>() << std::endl;
  std::cout << "BTree::[Set/Get]Type()... " << std::flush
    << TimeBranchAllocation<BTree>() << std::endl;
  
  std::cout << "BBTree::FindFree() [ordered]... " << std::flush
    << TimeOrderedAllocation<BBTree>() << std::endl;
  std::cout << "BTree::FindFree() [ordered]... " << std::flush
    << TimeOrderedAllocation<BTree>() << std::endl;
  
  std::cout << "BBTree::FindFree() [single-page]... " << std::flush
    << TimeBaseAllocation<BBTree>() << std::endl;
  std::cout << "BTree::FindFree() [single-page]... " << std::flush
    << TimeBaseAllocation<BTree>() << std::endl;
  
  std::cout << "BBTree::[Alloc/Dealloc]() [leaf]... " << std::flush
    << TimeLeafAllocation<BBTree>() << std::endl;
  std::cout << "BTree::[Alloc/Dealloc]() [leaf]... " << std::flush
    << TimeLeafAllocation<BTree>() << std::endl;
  
  std::cout << "BBTree::FindByShadow()... " << std::flush
    << TimeFindByShadow<BBTree>() << std::endl;
  std::cout << "BTree::FindByShadow()... " << std::flush
    << TimeFindByShadow<BTree>() << std::endl;
  
  std::cout << "BBTree::FindAligned() [loose]... " << std::flush
    << TimeLooseAlignment<BBTree>() << std::endl;
  std::cout << "BTree::FindAligned() [loose]... " << std::flush
    << TimeLooseAlignment<BTree>() << std::endl;
  
  std::cout << "BBTree::FindAligned() [tight]... " << std::flush
    << TimeTightAlignment<BBTree>() << std::endl;
  std::cout << "BTree::FindAligned() [tight]... " << std::flush
    << TimeTightAlignment<BTree>() << std::endl;
  
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
    assert(p == Path::Root());
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
  AllocAllButMiddle(tree, Path::Root());
  
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

template <class T>
uint64_t TimeLeafAllocation() {
  static const int depth = 15;
  uint8_t * buf = new uint8_t[T::MemorySize(depth)];
  T tree(depth, buf);
  uint64_t start = Microtime();
  
  // allocate every single base node
  for (int i = 0; i < (1 << (depth - 1)); i++) {
    Path p;
    bool res = tree.Alloc(depth - 1, p);
    assert(res);
    assert(p.GetDepth() == depth - 1);
  }
  
  // assert that there are no base nodes left
  Path noP;
  assert(!tree.Alloc(depth - 1, noP));
  
  // free every base node
  for (uint64_t i = 0; i < (1 << (depth - 1)); i++) {
    tree.Dealloc(Path(depth - 1, i));
  }
  
  assert(tree.GetType(Path::Root()) == NodeTypeFree);
  
  uint64_t result = Microtime() - start;
  delete buf;
  return result;
}

template <class T>
uint64_t TimeFindByShadow() {
  static const int depth = 17; // must be more than 12
  uint8_t * buf = new uint8_t[T::MemorySize(depth)];
  T tree(depth, buf);
  
  uint64_t sum = 0;
  
  // allocate the root node
  Path p;
  bool res = tree.Alloc(0, p);
  assert(res);
  assert(p == Path::Root());
  
  // attempt to find root node (many times)
  uint64_t start = Microtime();
  for (int i = 0; i < 0x10000; i++) {
    bool res = tree.FindByShadow(i, p);
    assert(res);
    assert(p == Path::Root());
  }
  sum += Microtime() - start;
  
  // allocate the leftmost node
  tree.Carve(Path::Root(), 0, 1);
  
  // attempt to find first shadow node (many times)
  start = Microtime();
  for (int i = 0; i < 0x10000; i++) {
    tree.FindByShadow(0, p);
  }
  sum += Microtime() - start;
  
  delete buf;
  return sum;
}

template <class T>
uint64_t TimeLooseAlignment() {
  static const int depth = 17;
  uint8_t * buf = new uint8_t[T::MemorySize(depth)];
  T tree(depth, buf);
  
  Path expectedFree(depth - 2, 0);
  
  // setup the tree to have one two-chunk free block on the left, and then
  // badly aligned chunks everywhere else
  for (int i = 0; i < depth - 1; i++) {
    for (uint64_t j = 0; j < Path::DepthCount(i); j++) {
      Path p(i, j);
      tree.SetType(p, NodeTypeContainer);
    }
  }
  tree.SetType(expectedFree, NodeTypeFree);
  for (uint64_t j = 2; j < Path::DepthCount(depth - 1); j += 2) {
    Path p(depth - 1, j);
    tree.SetType(p, NodeTypeData);
  }
  
  uint64_t sum = 0;
  uint64_t start = Microtime();
  for (int i = 0; i < 0x1000; i++) {
    Path p;
    bool res = tree.FindAligned(depth - 1, depth - 2, p);
    assert(res);
    assert(p == expectedFree);
  }
  sum += Microtime() - start;
  
  // put free guy on the right
  expectedFree = Path(depth - 2, Path::DepthCount(depth - 2) - 1);
  tree.SetType(Path(depth - 2, 0), NodeTypeContainer);
  tree.SetType(Path(depth - 1, 0), NodeTypeData);
  tree.SetType(expectedFree, NodeTypeFree);
  start = Microtime();
  for (int i = 0; i < 0x1000; i++) {
    Path p;
    bool res = tree.FindAligned(depth - 1, depth - 2, p);
    assert(res);
    assert(p == expectedFree);
  }
  sum += Microtime() - start;
  
  delete buf;
  return sum;
}

template <class T>
uint64_t TimeTightAlignment() {
  static const int depth = 17;
  uint8_t * buf = new uint8_t[T::MemorySize(depth)];
  T tree(depth, buf);
  
  Path expectedFree(depth - 1, 0);
  
  // setup the tree to have one free chunk on the far left, and then badly
  // aligned chunks everywhere else
  for (int i = 0; i < depth - 1; i++) {
    for (uint64_t j = 0; j < Path::DepthCount(i); j++) {
      Path p(i, j);
      tree.SetType(p, NodeTypeContainer);
    }
  }
  tree.SetType(expectedFree.Sibling(), NodeTypeData);
  for (uint64_t j = 2; j < Path::DepthCount(depth - 1); j += 2) {
    Path p(depth - 1, j);
    tree.SetType(p, NodeTypeData);
  }
  
  uint64_t sum = 0;
  uint64_t start = Microtime();
  for (int i = 0; i < 0x1000; i++) {
    Path p;
    bool res = tree.FindAligned(depth - 1, depth - 2, p);
    assert(res);
    assert(p == expectedFree);
  }
  sum += Microtime() - start;
  
  // put free guy on the right
  expectedFree = Path(depth - 1, Path::DepthCount(depth - 1) - 2);
  tree.SetType(Path(depth - 1, 0), NodeTypeData);
  tree.SetType(Path(depth - 1, 1), NodeTypeFree);
  tree.SetType(expectedFree, NodeTypeFree);
  tree.SetType(expectedFree.Sibling(), NodeTypeData);
  start = Microtime();
  for (int i = 0; i < 0x1000; i++) {
    Path p;
    bool res = tree.FindAligned(depth - 1, depth - 2, p);
    assert(res);
    assert(p == expectedFree);
  }
  sum += Microtime() - start;
  
  delete buf;
  return sum;
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
