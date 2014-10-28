#include <iostream>
#include <analloc2/free-list>
#include "nanotime.hpp"
#include "posix-virtual-aligner.hpp"
#include "stack-allocator.hpp"

using namespace analloc;

PosixVirtualAligner aligner;
static const size_t RegionSize = sizeof(FreeListAllocator<size_t>::FreeRegion);

uint64_t ProfileFreeListEnd(size_t length, size_t iters);
bool HandleFailure(FreeListAllocator<size_t> *);

int main() {
  for (size_t i = 0; i < 13; ++i) {
    size_t len = 1 << i;
    std::cout << "FreeListAligner (" << len << " regions)..."
      << std::flush << " " << ProfileFreeListEnd(len, 100000)
      << std::endl;
  }
}

uint64_t ProfileFreeListEnd(size_t length, size_t iters) {
  assert(ansa::IsPowerOf2(length));
  
  StackAllocator<RegionSize> stack(length + 1, aligner);
  FreeListAligner<size_t> aligner(stack, HandleFailure);
  
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

bool HandleFailure(FreeListAllocator<size_t> *) {
  std::cerr << "allocation failure!" << std::endl;
  abort();
}
