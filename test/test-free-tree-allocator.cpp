#include "scoped-pass.hpp"
#include "posix-virtual-aligner.hpp"
#include <analloc2/free-tree>

using namespace analloc;

PosixVirtualAligner aligner;
typedef FreeTreeAllocator<AvlTree, uint16_t, uint8_t> AllocatorClass;

void TestFullRegion();
void TestPartialRegion();
void TestJoins();
bool HandleFailure(AllocatorClass *);

int main() {
  TestFullRegion();
  assert(aligner.GetAllocCount() == 0);
  TestPartialRegion();
  assert(aligner.GetAllocCount() == 0);
  TestJoins();
  assert(aligner.GetAllocCount() == 0);
  return 0;
}

void TestFullRegion() {
  ScopedPass pass("FreeTreeAllocator [full-region]");
  AllocatorClass allocator(aligner, HandleFailure);
  
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

void TestPartialRegion() {
  ScopedPass pass("FreeTreeAllocator [partial]");
  AllocatorClass allocator(aligner, HandleFailure);
  uint16_t addr;
  
  allocator.Dealloc(0x100, 0x10);
  for (int i = 0; i < 0x10; ++i) {
    assert(allocator.Alloc(addr, 1));
    assert(addr == (uint16_t)i + 0x100);
  }
  assert(!allocator.Alloc(addr, 1));
  
  allocator.Dealloc(0x100, 0x10);
  allocator.Dealloc(0x120, 0x10);
  allocator.Dealloc(0x140, 0x10);
  
  assert(allocator.Alloc(addr, 0x8));
  assert(addr == 0x100);
  assert(allocator.Alloc(addr, 0x9));
  assert(addr == 0x120);
  assert(allocator.Alloc(addr, 0x10));
  assert(addr == 0x140);
  assert(allocator.Alloc(addr, 0x8));
  assert(addr == 0x108);
  assert(allocator.Alloc(addr, 0x7));
  assert(addr == 0x129);
  assert(!allocator.Alloc(addr, 1));
  
  // Deallocate a couple regions so that the main() function can test if the
  // destructor properly frees regions.
  allocator.Dealloc(0x100, 1);
  allocator.Dealloc(0x102, 1);
}

void TestJoins() {
  ScopedPass pass("FreeTreeAllocator [joins]");
  
  AllocatorClass allocator(aligner, HandleFailure);
  uint16_t addr;
  
  // Joining a middle region to two outer regions
  allocator.Dealloc(0x100, 0x10);
  allocator.Dealloc(0x120, 0x10);
  allocator.Dealloc(0x110, 0x10);
  assert(allocator.Alloc(addr, 0x30));
  assert(addr == 0x100);
  assert(!allocator.Alloc(addr, 1));
  
  // Joining a region with it's previous region (with another further region)
  allocator.Dealloc(0x100, 0x10);
  allocator.Dealloc(0x120, 0x10);
  allocator.Dealloc(0x110, 0xf);
  assert(allocator.Alloc(addr, 0x1f));
  assert(addr == 0x100);
  assert(allocator.Alloc(addr, 0x10));
  assert(addr == 0x120);
  assert(!allocator.Alloc(addr, 1));
  
  // Joining a region with the next region (with a distant region below it)
  allocator.Dealloc(0x100, 0xf);
  allocator.Dealloc(0x120, 0x10);
  allocator.Dealloc(0x110, 0x10);
  assert(allocator.Alloc(addr, 0x20));
  assert(addr == 0x110);
  assert(allocator.Alloc(addr, 0xf));
  assert(addr == 0x100);
  assert(!allocator.Alloc(addr, 1));
  
  // Joining a region with it's previous region (no distant region)
  allocator.Dealloc(0x100, 0x10);
  allocator.Dealloc(0x110, 0xf);
  assert(allocator.Alloc(addr, 0x1f));
  assert(addr == 0x100);
  assert(!allocator.Alloc(addr, 1));
  
  // Joining a region with it's next region (no distant region)
  allocator.Dealloc(0x110, 0xf);
  allocator.Dealloc(0x100, 0x10);
  assert(allocator.Alloc(addr, 0x1f));
  assert(addr == 0x100);
  assert(!allocator.Alloc(addr, 1));
}

bool HandleFailure(FreeTreeAllocator<AvlTree, uint16_t, uint8_t> *) {
  std::cerr << "HandleFailure()" << std::endl;
  abort();
}
