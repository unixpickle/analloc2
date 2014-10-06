// NOTE: this test ought to be taken with a grain of salt, given that the
// malloc() and free() functions may take up a significant amount of the
// benchmark.

#include <iostream>
#include <analloc2>
#include "nanotime.hpp"
#include "posix-virtual-aligner.hpp"

using namespace analloc;

uint64_t ProfileFreeListEnd(size_t length, size_t iters);
bool HandleFailure(FreeListAllocator<size_t> *);

int main() {
  for (size_t i = 0; i < 5; ++i) {
    size_t len = 0x100 << i;
    std::cout << "FreeListAllocator (" << len << " regions)..."
      << std::flush << " " << ProfileFreeListEnd(len, 100000) << std::endl;
  }
}

uint64_t ProfileFreeListEnd(size_t length, size_t iterations) {
  PosixVirtualAligner aligner;
  FreeListAllocator<size_t> allocator(aligner, HandleFailure);
  
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

bool HandleFailure(FreeListAllocator<size_t> *) {
  std::cerr << "allocation failure!" << std::endl;
  abort();
}
