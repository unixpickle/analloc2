#include "scoped-pass.hpp"
#include "posix-virtual-aligner.hpp"
#include <analloc2/free-list>
#include <cassert>

using namespace analloc;

PosixVirtualAligner posixAligner;

void TestSplitCases();
void TestOffsetAlign();
void TestEmptyAlign();

template <typename T>
bool HandleFailure(T *);

int main() {
  TestSplitCases();
  assert(posixAligner.GetAllocCount() == 0);
  TestOffsetAlign();
  assert(posixAligner.GetAllocCount() == 0);
  TestEmptyAlign();
  assert(posixAligner.GetAllocCount() == 0);
  return 0;
}

void TestSplitCases() {
  ScopedPass pass("FreeListAligner [split cases]");
  FreeListAligner<uint16_t, uint8_t> aligner(posixAligner, HandleFailure);
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

void TestOffsetAlign() {
  ScopedPass pass("FreeListAligner [offset align]");
  FreeListAligner<uint16_t, uint8_t> aligner(posixAligner, HandleFailure);
  uint16_t addr;
  
  aligner.Dealloc(0xf, 1);
  assert(!aligner.Align(addr, 0x10, 1));
  assert(aligner.OffsetAlign(addr, 0x10, 1, 1));
  assert(addr == 0xf);
  
  aligner.Dealloc(0x101, 0x10);
  // Align from the end of the chunk
  assert(aligner.Align(addr, 0x10, 1));
  assert(addr == 0x110);
  assert(!aligner.Align(addr, 0x10, 1));
  assert(posixAligner.GetAllocCount() == 1);
  // Align from the beginning of the chunk
  assert(aligner.OffsetAlign(addr, 0x100, 0xff, 1));
  assert(addr == 0x101);
  assert(posixAligner.GetAllocCount() == 1);
  // Align from the middle of the chunk
  assert(aligner.OffsetAlign(addr, 0x100, 0xfb, 1));
  assert(addr == 0x105);
  assert(posixAligner.GetAllocCount() == 2);
}

void TestEmptyAlign() {
  ScopedPass pass("FreeListAligner [empty align]");
  FreeListAligner<uint8_t> aligner(posixAligner, HandleFailure);
  uint8_t addr;
  
  aligner.Dealloc(8, 8);
  assert(!aligner.Align(addr, 0x10, 1));
  assert(aligner.Align(addr, 0x10, 0));
  assert(addr == 0x10);
  assert(aligner.Alloc(addr, 8));
  assert(addr == 8);
}

template <typename T>
bool HandleFailure(T *) {
  std::cerr << "allocation failure!" << std::endl;
  abort();
}
