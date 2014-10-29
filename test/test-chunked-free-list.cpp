#include "scoped-pass.hpp"
#include "posix-virtual-aligner.hpp"
#include <analloc2/free-list>

using namespace analloc;

PosixVirtualAligner posixAligner;

void TestAlloc();
void TestAlign();

template <typename T>
bool HandleFailure(T *);

int main() {
  TestAlloc();
  assert(posixAligner.GetAllocCount() == 0);
  TestAlign();
  assert(posixAligner.GetAllocCount() == 0);
  return 0;
}

void TestAlloc() {
  ScopedPass pass("ChunkedFreeList::Alloc()");
  
  ChunkedFreeList<uint8_t> allocator(0x10, posixAligner, HandleFailure);
  
  uint8_t addr;
  assert(!allocator.Alloc(addr, 1));
  allocator.Dealloc(0x20, 1);
  assert(allocator.Alloc(addr, 0xf));
  assert(addr == 0x20);
  assert(!allocator.Alloc(addr, 1));
  allocator.Dealloc(0x10, 0x1f);
  assert(allocator.Alloc(addr, 0x20));
  assert(addr == 0x10);
  allocator.Dealloc(0xf0, 1);
  assert(allocator.Alloc(addr, 0x10));
  assert(addr == 0xf0);
}

void TestAlign() {
  ScopedPass pass("ChunkedFreeList::Align()");
  
  ChunkedFreeList<uint8_t> aligner(0x10, posixAligner, HandleFailure);
  
  uint8_t addr;
  assert(!aligner.Align(addr, 1, 1));
  assert(!aligner.Align(addr, 0x10, 1));
  assert(!aligner.Align(addr, 0, 0));
  
  aligner.Dealloc(0x10, 1);
  assert(aligner.Align(addr, 8, 1));
  assert(addr == 0x10);
  assert(posixAligner.GetAllocCount() == 0);
  
  aligner.Dealloc(0x10, 0x20);
  assert(aligner.Align(addr, 0x20, 1));
  assert(addr == 0x20);
  assert(!aligner.OffsetAlign(addr, 0x10, 0x8, 1));
  assert(aligner.OffsetAlign(addr, 0x10, 0x10, 1));
  assert(addr == 0x10);
  assert(!aligner.Align(addr, 1, 1));
}

template <typename T>
bool HandleFailure(T *) {
  std::cerr << "allocation failure!" << std::endl;
  abort();
}
