#include "scoped-pass.hpp"
#include "posix-virtual-aligner.hpp"
#include <analloc2>

using namespace analloc;

PosixVirtualAligner aligner;

void TestFullRegion();
bool HandleFailure(FreeTreeAllocator<AvlTree, uint16_t, uint8_t> *);

int main() {
  TestFullRegion();
  return 0;
}

void TestFullRegion() {
  ScopedPass pass("FreeTreeAllocator [full-region]");
  FreeTreeAllocator<AvlTree, uint16_t, uint8_t> allocator(aligner,
                                                          HandleFailure);
  
  allocator.Dealloc(0x100, 0x10);
  allocator.Dealloc(0x111, 0x1);
  allocator.Dealloc(0x120, 0x10);
  
  uint16_t result;
  assert(allocator.Alloc(result, 0x10));
  assert(result == 0x100);
  assert(allocator.Alloc(result, 0x10));
  assert(result == 0x120);
  assert(!allocator.Alloc(result, 0x10));
  assert(allocator.Alloc(result, 0x1));
  assert(result == 0x111);
}

bool HandleFailure(FreeTreeAllocator<AvlTree, uint16_t, uint8_t> *) {
  std::cerr << "HandleFailure()" << std::endl;
  abort();
}
