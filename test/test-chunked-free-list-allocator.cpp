#include "scoped-pass.hpp"
#include "posix-virtual-aligner.hpp"
#include <analloc2/free-list>

using namespace analloc;

PosixVirtualAligner aligner;

void TestGeneral();

template <typename T>
bool HandleFailure(T *);

int main() {
  TestGeneral();
  assert(aligner.GetAllocCount() == 0);
  return 0;
}

void TestGeneral() {
  ScopedPass pass("ChunkedFreeListAllocator [general]");
  
  ChunkedFreeListAllocator<uint8_t> allocator(0x10, aligner, 
      HandleFailure);
  
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

template <typename T>
bool HandleFailure(T *) {
  std::cerr << "allocation failure!" << std::endl;
  abort();
}
