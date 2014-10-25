#include "scoped-pass.hpp"
#include <analloc2/bitmap>

using namespace analloc;

void TestLargeAlignment();
void TestUnitAlignment();
void TestOverflowAlignment();
void TestLastUnitAlignment();
void TestSimpleOffsetAlignment();
void TestMultitypeOffsetAlignment();

int main() {
  TestLargeAlignment();
  TestUnitAlignment();
  TestOverflowAlignment();
  TestLastUnitAlignment();
  TestSimpleOffsetAlignment();
  TestMultitypeOffsetAlignment();
  return 0;
}

void TestLargeAlignment() {
  ScopedPass pass("BitmapAligner [large]");
  
  uint16_t address;
  uint16_t units[8];
  BitmapAligner<uint16_t, uint16_t, uint8_t> aligner(units, 0x80);
  
  // Alignment by a number which exceeds SizeType
  assert(aligner.Align(address, 0x1000, 1));
  assert(address == 0);
  assert(!aligner.Align(address, 0x1000, 1));
  assert(!aligner.Align(address, 0x80, 1));
  aligner.Dealloc(0, 1);
  assert(!aligner.Align(address, 0x1000, 0x81));
  assert(aligner.Align(address, 0x1000, 0x80));
  assert(address == 0);
}

void TestUnitAlignment() {
  ScopedPass pass("BitmapAligner [unit]");
  
  uint16_t address;
  uint16_t units[8];
  BitmapAligner<uint16_t, uint16_t, uint8_t> aligner(units, 0x80);
  
  for (uint16_t align = 0; align < 2; ++align) {
    // Make sure that, indeed, alignments <= 1 result in normal allocations
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
}

void TestOverflowAlignment() {
  ScopedPass pass("BitmapAligner [overflow]");
  
  uint16_t address;
  uint16_t units[8];
  BitmapAligner<uint16_t, uint16_t, uint8_t> aligner(units, 0x80);
  
  assert(aligner.Align(address, 1, 0x40));
  assert(address == 0);
  assert(aligner.Align(address, 0x20, 0x20));
  assert(address == 0x40);
  aligner.Dealloc(0x50, 0x10);
  assert(!aligner.Align(address, 0x40, 0x10));
  assert(aligner.Align(address, 0x10, 0x10));
  assert(address == 0x50);
  assert(!aligner.Align(address, 0x80, 0x20));
  assert(aligner.Align(address, 0x20, 0x20));
  assert(address == 0x60);
  assert(!aligner.Align(address, 1, 1));
}

void TestLastUnitAlignment() {
  ScopedPass pass("BitmapAligner [last unit]");
  
  uint16_t address;
  uint16_t units[0x10];
  BitmapAligner<uint16_t, uint16_t, uint8_t> aligner(units, 0xff);
  
  assert(aligner.Align(address, 0x80, 1));
  assert(address == 0);
  assert(aligner.Align(address, 0x80, 1));
  assert(address == 0x80);
  assert(!aligner.Align(address, 0x80, 1));
}

void TestSimpleOffsetAlignment() {
  ScopedPass pass("BitmapAligner [simple offset]");
  
  uint8_t address;
  uint8_t units[2];
  BitmapAligner<uint8_t, uint8_t> aligner(units, 0x10);
  
  assert(!aligner.OffsetAlign(address, 0x10, 1, 2));
  assert(aligner.OffsetAlign(address, 0x10, 1, 1));
  assert(address == 0xf);
  assert(!aligner.OffsetAlign(address, 0x10, 1, 1));
  
  assert(!aligner.OffsetAlign(address, 0x10, 5, 5));
  assert(aligner.OffsetAlign(address, 0x10, 5, 1));
  assert(address == 0xb);
  assert(!aligner.OffsetAlign(address, 0x10, 5, 1));
  
  assert(aligner.OffsetAlign(address, 4, 1, 1));
  assert(address == 3);
  assert(aligner.OffsetAlign(address, 4, 1, 2));
  assert(address == 7);
}

void TestMultitypeOffsetAlignment() {
  ScopedPass pass("BitmapAligner [multitype offset]");
  
  uint16_t address;
  uint8_t units[2];
  BitmapAligner<uint8_t, uint16_t, uint8_t> aligner(units, 0x10);
  
  assert(!aligner.OffsetAlign(address, 0x200, 1, 1));
  assert(!aligner.OffsetAlign(address, 0x200, 0x201, 1));
  assert(aligner.OffsetAlign(address, 0x200, 0x200, 1));
  assert(address == 0);
  assert(!aligner.OffsetAlign(address, 0x200, 0x200, 1));
  assert(aligner.OffsetAlign(address, 0x200, 0x1f8, 1));
  assert(address == 0x8);
  assert(!aligner.OffsetAlign(address, 0x200, 0x1f8, 1));

  // Wrap-around alignments
  assert(aligner.OffsetAlign(address, 0x100, 0xfffe, 1));
  assert(address == 2);
  assert(!aligner.OffsetAlign(address, 0x100, 0xffff, 2));
  assert(aligner.OffsetAlign(address, 0x100, 0xffff, 1));
  assert(address == 1);
}
