#include "scoped-pass.hpp"
#include <analloc2>

using namespace analloc;

void TestTrivialCases();
void TestSimpleOffset();
void TestMultitypeOffset();
void TestSimpleScaled();
void TestMultitypeScaled();

int main() {
  TestTrivialCases();
  TestSimpleOffset();
  TestMultitypeOffset();
  TestSimpleScaled();
  TestMultitypeScaled();
  return 0;
}

void TestTrivialCases() {
  ScopedPass pass("TransformedBitmapAligner [trivial]");
  
  uint8_t bmData[2];
  TransformedBitmapAligner<uint8_t, uint8_t> aligner(1, 0, bmData, 16);
  
  uint8_t address;
  
  // Make sure that, indeed, alignments <= 1 result in normal allocations
  for (uint16_t align = 0; align < 2; ++align) {
    assert(aligner.Align(address, align, 1));
    assert(address == 0);
    assert(aligner.Align(address, align, 1));
    assert(address == 1);
    assert(aligner.Align(address, align, 4));
    assert(address == 2);
    aligner.Dealloc(1, 1);
    assert(aligner.Align(address, align, 2));
    assert(address == 6);
    assert(aligner.Align(address, align, 1));
    assert(address == 1);
    aligner.Dealloc(0, 8);
  }
  
  // Test what happens when alignments overflow
  assert(aligner.Align(address, 0x80, 1));
  assert(address == 0);
  assert(!aligner.Align(address, 0x80, 1));
  assert(aligner.Align(address, 8, 1));
  assert(address == 8);
  assert(!aligner.Align(address, 8, 1));
  assert(aligner.Align(address, 4, 1));
  assert(address == 4);
  assert(aligner.Align(address, 4, 1));
  assert(address == 0xc);
  assert(!aligner.Align(address, 4, 1));
  aligner.Dealloc(1, 1);
  aligner.Dealloc(4, 1);
  aligner.Dealloc(8, 1);
  aligner.Dealloc(0xc, 1);
}

void TestSimpleOffset() {
  ScopedPass pass("TransformedBitmapAligner [simple offset]");
  
  uint8_t bmData[2];
  TransformedBitmapAligner<uint8_t, uint8_t> aligner(1, 3, bmData, 14);
  
  uint8_t address;
  
  // Try allocating the last byte, at 0x10
  assert(!aligner.Align(address, 0x20, 1));
  assert(!aligner.Align(address, 0x10, 2));
  assert(aligner.Align(address, 0x10, 1));
  assert(address == 0x10);
  assert(!aligner.Align(address, 0x10, 1));
  
  // Try some internal alignments.
  assert(aligner.Align(address, 8, 5));
  assert(address == 8);
  assert(!aligner.Align(address, 8, 1));
  assert(aligner.Align(address, 4, 1));
  assert(address == 4);
  assert(!aligner.Align(address, 4, 1)); // 0xc is taken already!
}

void TestMultitypeOffset() {
  ScopedPass pass("TransformedBitmapAligner [multitype offset]");
  
  uint8_t bmData[2];
  TransformedBitmapAligner<uint8_t, uint16_t, uint8_t>
      aligner(1, 0x103, bmData, 14);
  
  uint16_t address;
  
  // Try allocating the last byte, at 0x110
  assert(!aligner.Align(address, 0x20, 1));
  assert(!aligner.Align(address, 0x10, 2));
  assert(aligner.Align(address, 0x10, 1));
  assert(address == 0x110);
  assert(!aligner.Align(address, 0x10, 1));
  
  // Try some internal alignments.
  assert(aligner.Align(address, 8, 5));
  assert(address == 0x108);
  assert(!aligner.Align(address, 8, 1));
  assert(aligner.Align(address, 4, 1));
  assert(address == 0x104);
  assert(!aligner.Align(address, 4, 1)); // 0x10c is taken already!
  
  // Test alignments by sizes that exceed the SizeType
  TransformedBitmapAligner<uint8_t, uint16_t, uint8_t>
    aligner2(1, 0x1ff, bmData, 14);
  assert(aligner2.Align(address, 0x200, 1));
  assert(address == 0x200);
  assert(aligner2.Align(address, 1, 1));
  assert(address == 0x1ff);
  assert(aligner2.Align(address, 8, 1));
  assert(address == 0x208);
  assert(!aligner2.Align(address, 8, 1));
}

void TestSimpleScaled() {
  ScopedPass pass("TransformedBitmapAligner [simple scaled]");
  
  uint8_t buffer[2];
  TransformedBitmapAligner<uint8_t, uint8_t> aligner(0x10, 0, buffer, 0xf);
  uint8_t address;
  
  assert(aligner.Align(address, 0x80, 0x20));
  assert(address == 0x0);
  assert(aligner.Align(address, 0x80, 0x41)); // take up to 0xd0
  assert(address == 0x80);
  assert(!aligner.Align(address, 0x80, 1));
  assert(aligner.Align(address, 0x40, 1));
  assert(address == 0x40);
  assert(!aligner.Align(address, 0x40, 1)); // 0xc0 is taken
}

void TestMultitypeScaled() {
  ScopedPass pass("TransformedBitmapAligner [multitype scaled]");
  
  uint8_t buffer[2];
  TransformedBitmapAligner<uint8_t, uint16_t, uint8_t>
      aligner(0x10, 0x210, buffer, 0x10);
  uint16_t address;
  
  assert(!aligner.Align(address, 0x200, 1));
  assert(!aligner.Align(address, 0x100, 0x11));
  assert(aligner.Align(address, 0x100, 0x10));
  assert(address == 0x300);
  assert(!aligner.Align(address, 0x100, 1));
  assert(!aligner.Align(address, 0x80, 0x81));
  assert(aligner.Align(address, 0x80, 1));
  assert(address == 0x280);
}
