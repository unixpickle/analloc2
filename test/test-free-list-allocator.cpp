#include "scoped-pass.hpp"
#include "posix-virtual-aligner.hpp"
#include <analloc2>

using namespace analloc;

void TestFullRegion();
void TestPartialRegion();
void TestJoins();
void HandleFailure(FreeListAllocator<uint16_t, uint8_t> *);

int main() {
  TestFullRegion();
  TestPartialRegion();
  TestJoins();
  return 0;
}

void TestFullRegion() {
  ScopedPass pass("FreeListAllocator [full-region]");
  
  PosixVirtualAligner aligner;
  FreeListAllocator<uint16_t, uint8_t> allocator(aligner, HandleFailure);
  uint16_t addr;
  
  allocator.Dealloc(0x100, 0x10);
  assert(allocator.Alloc(addr, 0x10));
  assert(addr == 0x100);
  assert(!allocator.Alloc(addr, 1));
  allocator.Dealloc(0x140, 0x20);
  allocator.Dealloc(0x120, 0x10);
  allocator.Dealloc(0x100, 0x10);
  assert(allocator.Alloc(addr, 0x10));
  assert(addr == 0x100);
  assert(allocator.Alloc(addr, 0x10));
  assert(addr == 0x120);
  assert(allocator.Alloc(addr, 0x20));
  assert(addr == 0x140);
  assert(!allocator.Alloc(addr, 1));
}

void TestPartialRegion() {
  ScopedPass pass("FreeListAllocator [partial]");
  
  PosixVirtualAligner aligner;
  FreeListAllocator<uint16_t, uint8_t> allocator(aligner, HandleFailure);
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
}

void TestJoins() {
  assert(false);
  ScopedPass pass("FreeListAllocator [joins]");
  
  PosixVirtualAligner aligner;
  FreeListAllocator<uint16_t, uint8_t> allocator(aligner, HandleFailure);
  uint16_t addr;
  
  // TODO: this
  
  // Joining a middle region to two outer regions
  allocator.Dealloc(0x100, 0x10);
}

void HandleFailure(FreeListAllocator<uint16_t, uint8_t> *) {
  std::cerr << "allocation failure!" << std::endl;
  abort();
}
