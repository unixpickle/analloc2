#include "scoped-pass.hpp"
#include "posix-virtual-aligner.hpp"
#include <analloc2/free-list>
#include <cassert>

using namespace analloc;

bool HandleFailure(FreeListAllocator<uint16_t, uint8_t> *);

int main() {
  ScopedPass pass("FreeListAligner");
  
  PosixVirtualAligner posixAligner;
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
  
  assert(posixAligner.GetAllocCount() == 0);
  
  return 0;
}

bool HandleFailure(FreeListAllocator<uint16_t, uint8_t> *) {
  std::cerr << "allocation failure!" << std::endl;
  abort();
}
