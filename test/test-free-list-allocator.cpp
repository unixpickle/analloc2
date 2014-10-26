#include "scoped-pass.hpp"
#include "posix-virtual-aligner.hpp"
#include <analloc2/free-list>
#include <cassert>

using namespace analloc;

void TestFullRegion();
void TestPartialRegion();
void TestJoins();
void TestEmptyAlloc();
void TestOverflow();

template <typename T>
bool HandleFailure(T *);

PosixVirtualAligner aligner;

int main() {
  TestFullRegion();
  TestPartialRegion();
  TestJoins();
  TestEmptyAlloc();
  TestOverflow();
  assert(aligner.GetAllocCount() == 0);
  return 0;
}

void TestFullRegion() {
  ScopedPass pass("FreeListAllocator [full-region]");
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
  
  // Deallocate a couple regions so that the main() function can test if the
  // destructor properly frees regions.
  allocator.Dealloc(0x100, 1);
  allocator.Dealloc(0x102, 1);
}

void TestJoins() {
  ScopedPass pass("FreeListAllocator [joins]");
  
  FreeListAllocator<uint16_t, uint8_t> allocator(aligner, HandleFailure);
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

void TestEmptyAlloc() {
  ScopedPass pass("FreeListAllocator [empty]");
  FreeListAllocator<uint8_t> allocator(aligner, HandleFailure);
  
  uint8_t addr;
  assert(!allocator.Alloc(addr, 0));
  allocator.Dealloc(1, 1);
  
  assert(aligner.GetAllocCount() == 1);
  
  // Allocate a zero-sized buffer multiple times
  assert(allocator.Alloc(addr, 0));
  assert(addr == 1);
  assert(allocator.Alloc(addr, 0));
  assert(addr == 1);
  // Ensure that deallocating an empty buffer does nothing
  allocator.Dealloc(addr, 0);
  assert(allocator.Alloc(addr, 0));
  assert(addr == 1);
  
  assert(aligner.GetAllocCount() == 1);
  
  // Actually allocate the buffer
  assert(allocator.Alloc(addr, 1));
  assert(addr == 1);
  assert(aligner.GetAllocCount() == 0);
  // Make sure that zero-sized allocations are no longer possible
  assert(!allocator.Alloc(addr, 0));
  allocator.Dealloc(1, 0);
  assert(aligner.GetAllocCount() == 0);
  assert(!allocator.Alloc(addr, 0));
  // Actually deallocate the buffer
  allocator.Dealloc(1, 1);
  assert(aligner.GetAllocCount() == 1);
  // Ensure that zero-sized allocation works again
  assert(allocator.Alloc(addr, 0));
  assert(addr == 1);
  assert(aligner.GetAllocCount() == 1);
}

void TestOverflow() {
  ScopedPass pass("FreeListAllocator [overflow]");
  
  FreeListAllocator<uint8_t> allocator(aligner, HandleFailure);
  allocator.Dealloc(0x80, 0x10);
  allocator.Dealloc(0xf0, 0x10);
  uint8_t region;
  assert(allocator.Alloc(region, 8));
  assert(region == 0x80);
  assert(allocator.Alloc(region, 9));
  assert(region == 0xf0);
  assert(allocator.Alloc(region, 8));
  assert(region == 0x88);
  assert(allocator.Alloc(region, 7));
  assert(region == 0xf9);
  assert(!allocator.Alloc(region, 1));
  
  allocator.Dealloc(0xf0, 0x10);
  assert(allocator.Alloc(region, 0x10));
  assert(region == 0xf0);
  assert(!allocator.Alloc(region, 1));
}

template <typename T>
bool HandleFailure(T *) {
  std::cerr << "allocation failure!" << std::endl;
  abort();
}
