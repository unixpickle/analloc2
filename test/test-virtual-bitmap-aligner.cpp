#include "scoped-pass.hpp"
#include "scoped-buffer.hpp"
#include <analloc2>

using namespace analloc;

void TestConstructed(size_t pageSize, size_t pageCount, size_t headerSize);

int main() {
  for (int i = 0; i < 5; ++i) {
    size_t size = 1UL << i;
    size_t headerSize = ansa::Max<size_t>(sizeof(size_t), size);
    const size_t pageCount = 0x100;
    TestConstructed(size, pageCount, headerSize);
  }
  return 0;
}

void TestConstructed(size_t pageSize, size_t pageCount, size_t headerSize) {
  ScopedPass pass("VirtualBitmapAligner [constructed, ", pageSize, ", ",
                  pageCount, ", ", headerSize, "]");
  
  assert(headerSize >= pageSize);
  assert(ansa::IsAligned(headerSize, pageSize));
  
  uint8_t bitmap[ansa::RoundUpDiv<size_t>(pageCount, 8)];
  uint8_t zeroBitmap[sizeof(bitmap)];
  size_t dataSize = pageSize * pageCount;
  ScopedBuffer<uint8_t *> data(dataSize, dataSize);
  uintptr_t start = data;

  ansa::Bzero(zeroBitmap, sizeof(zeroBitmap));
  
  VirtualBitmapAligner<uint8_t> aligner(pageSize, start, bitmap, dataSize);
  assert(aligner.GetOffset() == start);
  assert(aligner.GetScale() == pageSize);
  assert(aligner.GetBitCount() == pageCount);
  assert(aligner.GetTotalSize() == pageCount * pageSize);
  
  uintptr_t addr;
  
  // Largest allocation possible, in the form of unit alignment
  assert(!aligner.Align(addr, 1, dataSize - headerSize + 1));
  assert(aligner.Align(addr, 1, dataSize - headerSize));
  assert(addr == start + headerSize);
  aligner.Dealloc(addr, dataSize - headerSize);
  
  // Largest allocation possible with headerSize alignment
  assert(aligner.Align(addr, headerSize, dataSize - headerSize));
  assert(addr == start + headerSize);
  aligner.Dealloc(addr, dataSize - headerSize);
  assert(!aligner.Align(addr, headerSize * 2, dataSize - headerSize));
  
  // Alignment which doesn't leave enough initial space for any substantial
  // memory.
  assert(aligner.Align(addr, headerSize * 2, 1));
  assert(addr == start + headerSize * 2);
  assert(aligner.Align(addr, 1, 1));
  assert(addr == start + pageSize + headerSize * 3);
  aligner.Free(addr);
  // Ensure that align and alloc do the same thing
  assert(aligner.Alloc(addr, 0));
  assert(addr == start + headerSize);
  aligner.Free(addr);
  assert(aligner.Align(addr, 1, 0));
  assert(addr == start + headerSize);
  aligner.Free(addr);
  aligner.Free(start + headerSize * 2);
  
  // Test OffsetAlign(), basic case.
  assert(aligner.OffsetAlign(addr, headerSize * 4, headerSize * 2, 0));
  assert(addr == start + headerSize * 2);
  aligner.Dealloc(addr, 0);
  
  if (pageSize > 1) {
    // Make sure OffsetAlign() doesn't work when the offset is impossible.
    assert(!aligner.OffsetAlign(addr, headerSize, 1, 0));
  }
}
