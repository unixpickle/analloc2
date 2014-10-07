#include <iostream>
#include <analloc2>
#include <ansa/numeric-info>
#include "nanotime.hpp"
#include "posix-virtual-aligner.hpp"
#include "stack-allocator.hpp"

using namespace ansa;
using namespace analloc;

PosixVirtualAligner aligner;

uint64_t ProfileContains(int depth);
uint64_t ProfileSequentialAdds(int count);
uint64_t ProfileSequentialRemoves(int count);

int main() {
  // TODO: check the maximum depth we can do on this platform
  for (int depth = 2; depth <= 20; depth += 2) {
    assert(aligner.GetAllocCount() == 0);
    std::cout << "AvlTree<int>::Contains() [depth = " << depth << "] ... "
      << std::flush << ProfileContains(depth) << " nanos" << std::endl;
  }
  for (int factor = 0; factor < 4; ++factor) {
    int count = 0x800 << factor;
    std::cout << "AvlTree<int>::Add() [sequential, " << count << "] ... "
      << std::flush << ProfileSequentialAdds(count) << std::endl;
    std::cout << "AvlTree<int>::Remove() [sequential, " << count << "] ... "
      << std::flush << ProfileSequentialRemoves(count) << std::endl;
  }
  return 0;
}

uint64_t ProfileContains(int depth) {
  assert((1L << depth) < ansa::NumericInfo<int>::max);
  AvlTree<int> tree(aligner);
  
  // Generate a completely balanced AVL tree of the given depth.
  for (int i = 0; i < depth; ++i) {
    int start = 1 << (depth - (i + 1));
    int inc = 2 * start;
    for (int j = 0; j < (1 << i); ++j) {
      tree.Add(start + (j * inc));
    }
  }
  assert(tree.GetRoot()->depth + 1 == depth);
  
  // Search for the bottom element in the tree a lot of times
  uint64_t start = Nanotime();
  const int iterations = 10000000;
  int element = 1;
  for (int i = 0; i < iterations; ++i) {
    // This may not work on all platforms; keep me posted. I do this so that
    // the optimizer doesn't omit the call to tree.Contains(...).
    __asm__ __volatile__("" : : "r" (tree.Contains(element)));
  }
  return (Nanotime() - start) / iterations;
}

uint64_t ProfileSequentialAdds(int count) {
  StackAllocator<sizeof(AvlTree<int>::Node)> stack(count, aligner);
  AvlTree<int> tree(stack);
  
  const int iterations = 100;
  uint64_t total = 0;
  for (int i = 0; i < iterations; ++i) {
    uint64_t start = Nanotime();
    for (int j = 0; j < count; ++j) {
      tree.Add(j);
    }
    total += Nanotime() - start;
    tree.Clear();
  }
  return total / iterations;
}

uint64_t ProfileSequentialRemoves(int count) {
  StackAllocator<sizeof(AvlTree<int>::Node)> stack(count, aligner);
  AvlTree<int> tree(stack);
  
  const int iterations = 100;
  uint64_t total = 0;
  for (int i = 0; i < iterations; ++i) {
    for (int j = 0; j < count; ++j) {
      tree.Add(j);
    }
    uint64_t start = Nanotime();
    for (int j = 0; j < count; ++j) {
      tree.Remove(j);
    }
    total += Nanotime() - start;
  }
  return total / iterations;
}