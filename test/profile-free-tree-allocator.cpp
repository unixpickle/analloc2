#include <iostream>
#include <analloc2/free-tree>
#include "nanotime.hpp"
#include "posix-virtual-aligner.hpp"
#include "stack-allocator.hpp"

using namespace analloc;

PosixVirtualAligner aligner;

template <template <class T> class Tree, template <class T> class Node>
uint64_t ProfileFreeTreeEnd(size_t length, size_t iters);

template <class T>
bool HandleFailure(T *);

int main() {
  for (size_t i = 0; i < 13; ++i) {
    size_t len = 1 << i;
    std::cout << "FreeTreeAllocator<AvlTree> (" << len << " regions) ... "
      << std::flush
      << ProfileFreeTreeEnd<AvlTree, AvlNode>(len, 100000) << std::endl;
  }
}

template <template <class T> class Tree, template <class T> class Node>
uint64_t ProfileFreeTreeEnd(size_t length, size_t iterations) {
  typedef Node<typename FreeTreeAllocator<Tree, size_t>::FreeRegion> Region;
  StackAllocator<sizeof(Region)> stack((length + 1) * 2, aligner);
  FreeTreeAllocator<Tree, size_t> allocator(stack, HandleFailure);
  
  // Carve out [length] regions of size 1, and then one final region of size 2.
  for (size_t i = 0; i < length; ++i) {
    allocator.Dealloc(i * 2, 1);
  }
  uint64_t start = Nanotime();
  size_t ignored = 0;
  for (size_t i = 0; i < iterations; ++i) {
    allocator.Dealloc(length * 2, 2);
    allocator.Alloc(ignored, 2);
    assert(ignored == length * 2);
  }
  return (Nanotime() - start) / iterations;
}

template <class T>
bool HandleFailure(T *) {
  std::cerr << "allocation failure!" << std::endl;
  abort();
}
