#include <iostream>
#include <analloc2/free-list>
#include "nanotime.hpp"
#include "posix-virtual-aligner.hpp"
#include "stack-allocator.hpp"

using namespace analloc;

PosixVirtualAligner aligner;
static const size_t RegionSize = sizeof(FreeList<size_t>::FreeRegion);

uint64_t ProfileFreeListAllocEnd(size_t length, size_t iters);
uint64_t ProfileFreeListAlignEnd(size_t length, size_t iters);

template <typename T>
bool HandleFailure(T *);

int main() {
  for (size_t i = 0; i < 13; ++i) {
    size_t len = 1 << i;
    std::cout << "FreeList::Alloc() [" << len << " regions]..."
      << std::flush << " " << ProfileFreeListAllocEnd(len, 100000)
      << std::endl;
    std::cout << "FreeList::Align() [" << len << " regions]..."
      << std::flush << " " << ProfileFreeListAlignEnd(len, 100000)
      << std::endl;
  }
}

uint64_t ProfileFreeListAllocEnd(size_t length, size_t iterations) {
  StackAllocator<RegionSize> stack(length + 1, aligner);
  FreeList<size_t> allocator(stack, HandleFailure);
  
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

uint64_t ProfileFreeListAlignEnd(size_t length, size_t iters) {
  assert(ansa::IsPowerOf2(length));
  
  StackAllocator<RegionSize> stack(length + 1, aligner);
  FreeList<size_t> aligner(stack, HandleFailure);
  
  // Carve out [length] regions of size 1, and then one final region of size 2.
  for (size_t i = 2; i < length; ++i) {
    aligner.Dealloc(i * 2, 1);
  }
  uint64_t start = Nanotime();
  size_t ignored = 0;
  for (size_t i = 0; i < iters; ++i) {
    aligner.Dealloc(length * 2, 1);
    aligner.Align(ignored, length * 2, 1);
    assert(ignored == length * 2);
  }
  (void)ignored;
  return (Nanotime() - start) / iters;
}

template <typename T>
bool HandleFailure(T *) {
  std::cerr << "allocation failure!" << std::endl;
  abort();
}
