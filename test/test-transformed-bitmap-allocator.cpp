#include "scoped-pass.hpp"
#include <analloc2>

using namespace analloc;

void TestScaled();
void TestOffset();
void TestScaledOffset();

int main() {
  TestScaled();
  TestOffset();
  TestScaledOffset();
  return 0;
}

void TestScaled() {
  ScopedPass pass("TransformedBitmapAllocator<uint8_t, uint16_t> [scaled]");
  
  // Create a bitmap allocator which is scaled by a factor of 3 and contains
  // 16 bits.
  uint8_t bitmap[2];
  TransformedBitmapAllocator<uint8_t, uint16_t> allocator(3, 0, bitmap, 16);
  
  assert(false);
}

void TestOffset() {
  ScopedPass pass("TransformedBitmapAllocator<uint8_t, uint16_t> [offset]");
  assert(false);
}

void TestScaledOffset() {
  ScopedPass pass("TransformedBitmapAllocator<uint8_t, uint16_t>");
  assert(false);
}
