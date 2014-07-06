#include "../src/bbtree.hpp"
#include "../src/btree.hpp"
#include "microtime.hpp"
#include <cstdint>
#include <iostream>

using namespace ANAlloc;

template <class T>
uint64_t TimeBranchAllocation();

template <class T>
uint64_t TimeOrderedAllocation();

template <class T>
uint64_t TimeBaseAllocation();

template <class T>
void FreeAll(T * tree, Path p = 0);

template <class T>
void AllocAllButMiddle(T * tree, Path p = 0);

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

template <class T>
uint64_t TimeBranchAllocation() {
  uint8_t * buf = new uint8_t[T::MemorySize(28)];
  T * tree = new T(28, buf);
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

template <class T>
uint64_t TimeOrderedAllocation() {
  uint8_t * buf = new uint8_t[T::MemorySize(28)];
  T * tree = new T(28, buf);
  uint64_t start = Microtime();
  
  for (int i = 0; i < 0x1000; i++) {
    for (int depth = 1; depth < 28; depth++) {
      Path p;
      tree->FindFree(depth, p);
      assert(PathDepth(p) == depth - 1);
      assert(tree->GetType(p) == Tree::NodeTypeFree);
      tree->SetType(p, Tree::NodeTypeContainer);
      tree->SetType(PathLeft(p), Tree::NodeTypeData);
      tree->SetType(PathRight(p), Tree::NodeTypeFree);
    }
    
    FreeAll(tree);
  }
  
  return Microtime() - start;
}

template <class T>
uint64_t TimeBaseAllocation() {
  uint8_t * buf = new uint8_t[T::MemorySize(12)];
  T * tree = new T(12, buf);
  AllocAllButMiddle(tree);
  
  uint64_t start = Microtime();
  
  for (int i = 0; i < 0x1000; i++) {
    Path p;
    bool res = tree->FindFree(tree->Depth() - 1, p);
    assert(res);
    tree->SetType(p, T::NodeTypeData);
    
    Path nonP;
    res = tree->FindFree(tree->Depth() - 1, nonP);
    assert(!res);
    
    assert(PathDepth(p) == tree->Depth() - 1);
    assert(PathIndex(p) == (1 << (tree->Depth() - 1)) / 2);
    tree->SetType(p, T::NodeTypeFree);
  }
  
  return Microtime() - start;
}

template <class T>
void FreeAll(T * tree, Path p) {
  auto type = tree->GetType(p);
  if (type == Tree::NodeTypeFree) {
    return;
  } else if (type == Tree::NodeTypeData) {
    tree->SetType(p, Tree::NodeTypeFree);
  } else {
    auto typeLeft = tree->GetType(PathLeft(p));
    auto typeRight = tree->GetType(PathRight(p));
    if (typeLeft != Tree::NodeTypeFree) {
      FreeAll(tree, PathLeft(p));
    }
    if (typeRight != Tree::NodeTypeFree) {
      FreeAll(tree, PathRight(p));
    }
    tree->SetType(p, Tree::NodeTypeFree);
  }
}

template <class T>
void AllocAllButMiddle(T * tree, Path p) {
  if (PathDepth(p) + 1 == tree->Depth()) {
    uint64_t rowCount = 1 << (tree->Depth() - 1);
    if (PathIndex(p) == rowCount / 2) {
      tree->SetType(p, T::NodeTypeFree);
    } else {
      tree->SetType(p, T::NodeTypeData);
    }
  } else {
    tree->SetType(p, T::NodeTypeContainer);
    AllocAllButMiddle(tree, PathLeft(p));
    AllocAllButMiddle(tree, PathRight(p));
  }
}
