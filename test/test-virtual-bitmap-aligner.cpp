#include "scoped-pass.hpp"
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
  uint8_t data[pageSize * pageCount];
  
  ansa::Bzero(zeroBitmap, sizeof(zeroBitmap));
  
  uintptr_t start = (uintptr_t)data;
  
  VirtualBitmapAligner<uint8_t> aligner(pageSize, start, bitmap, sizeof(data));
  assert(aligner.GetOffset() == start);
  assert(aligner.GetScale() == pageSize);
  
  // TODO: write real tests here.
  assert(false);
}
