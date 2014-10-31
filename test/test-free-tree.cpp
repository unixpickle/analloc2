#include "scoped-pass.hpp"
#include "posix-virtual-aligner.hpp"
#include <analloc2/free-tree>

using namespace analloc;

PosixVirtualAligner posixAligner;
typedef FreeTree<AvlTree, uint16_t, uint8_t> AllocatorClass;

void TestFullRegion();
void TestPartialRegion();
void TestJoins();
void TestSplits();

template <typename T>
bool HandleFailure(T *);

int main() {
  TestFullRegion();
  assert(posixAligner.GetAllocCount() == 0);
  TestPartialRegion();
  assert(posixAligner.GetAllocCount() == 0);
  TestJoins();
  assert(posixAligner.GetAllocCount() == 0);
  TestSplits();
  assert(posixAligner.GetAllocCount() == 0);
  return 0;
}

void TestFullRegion() {
  ScopedPass pass("FreeTree::Alloc() [full-region]");
  AllocatorClass allocator(posixAligner, HandleFailure);
  
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
  ScopedPass pass("FreeTree::Alloc() [partial]");
  AllocatorClass allocator(posixAligner, HandleFailure);
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
  ScopedPass pass("FreeTree::Alloc() [joins]");
  
  AllocatorClass allocator(posixAligner, HandleFailure);
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

void TestSplits() {
  ScopedPass pass("FreeTree::Align() [splits]");
  
  AllocatorClass aligner(posixAligner, HandleFailure);
  uint16_t addr;
  
  // Test when the region doesn't have enough room
  aligner.Dealloc(0, 0x20);
  assert(!aligner.Align(addr, 0x100, 0x21));
  assert(aligner.Alloc(addr, 0x1));
  assert(addr == 0x0);
  assert(!aligner.Align(addr, 0x2, 0x1f));
  assert(aligner.Alloc(addr, 0x1f));
  assert(addr == 0x1);
  
  // There is exactly enough room in this region and offset = 0
  aligner.Dealloc(0x10, 0x20);
  assert(aligner.Align(addr, 8, 0x20));
  assert(addr == 0x10);
  assert(!aligner.Alloc(addr, 1));
  
  // More than enough room with offset = 0
  aligner.Dealloc(0x10, 0x20);
  assert(aligner.Align(addr, 8, 0x10));
  assert(addr == 0x10);
  assert(aligner.Alloc(addr, 0x10));
  assert(addr == 0x20);
  assert(!aligner.Alloc(addr, 1));
  
  // Just enough room with offset != 0
  aligner.Dealloc(0xf, 0x21);
  assert(aligner.Align(addr, 0x10, 0x20));
  assert(addr == 0x10);
  assert(aligner.Alloc(addr, 1));
  assert(addr == 0xf);
  assert(!aligner.Alloc(addr, 1));
  
  // More than enough room with offset != 0
  aligner.Dealloc(0xf, 0x21);
  assert(aligner.Align(addr, 0x10, 0x10));
  assert(addr == 0x10);
  assert(aligner.Alloc(addr, 1));
  assert(addr == 0xf);
  assert(aligner.Alloc(addr, 0x10));
  assert(addr == 0x20);
  assert(!aligner.Alloc(addr, 1));
}

template <typename T>
bool HandleFailure(T *) {
  std::cerr << "HandleFailure()" << std::endl;
  abort();
}
