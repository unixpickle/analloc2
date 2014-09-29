#include "scoped-pass.hpp"
#include <analloc2>

using namespace analloc;

int main() {
  ScopedPass pass("BitmapAligner");
  
  uint16_t address;
  uint16_t units[0x10];
  BitmapAligner<uint16_t, uint16_t> aligner(0xf, units, 0x100);
  
  assert(aligner.Align(address, 0, 1));
  assert(address == 0xf);
  aligner.Dealloc(address, 1);
  assert(aligner.Align(address, 0x10, 0x20));
  assert(address == 0x10);
  assert(aligner.Align(address, 1, 1));
  assert(address == 0xf);
  assert(aligner.Align(address, 0x40, 0x40));
  assert(address == 0x40);
  assert(aligner.Align(address, 0x10, 0x20));
  assert(address == 0x80);
  assert(aligner.Align(address, 0x10, 0x10));
  assert(address == 0x30);
  assert(!aligner.Align(address, 0x100, 0x10));
  assert(aligner.Align(address, 0x100, 0xf));
  assert(address == 0x100);
  assert(aligner.Align(address, 0x50, 0x60));
  assert(address == 0xa0);
  assert(!aligner.Alloc(address, 1));
  
  return 0;
}
