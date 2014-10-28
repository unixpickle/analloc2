#include "scoped-pass.hpp"
#include "posix-virtual-aligner.hpp"
#include <analloc2/free-list>

using namespace analloc;

PosixVirtualAligner posixAligner;

void TestGeneral();

template <typename T>
bool HandleFailure(T *);

int main() {
  TestGeneral();
  return 0;
}

void TestGeneral() {
  ScopedPass pass("ChunkedFreeListAligner [general]");
  
  ChunkedFreeListAligner<uint8_t> aligner(0x10, posixAligner, HandleFailure);
  
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
  std::cerr << "allocation failed!" << std::endl;
  abort();
}