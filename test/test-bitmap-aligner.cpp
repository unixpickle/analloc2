#include "scoped-pass.hpp"
#include <analloc2>

using namespace analloc;

void TestLargeAlignment();
void TestUnitAlignment();
void TestOverflowAlignment();

int main() {
  TestLargeAlignment();
  TestUnitAlignment();
  TestOverflowAlignment();
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
