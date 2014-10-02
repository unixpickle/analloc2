#include "scoped-pass.hpp"
#include <analloc2>

using namespace analloc;

int main() {
  ScopedPass pass("BitmapAligner");
  
  uint16_t address;
  uint16_t units[0x10];
  BitmapAligner<uint16_t, uint16_t> aligner(units, 0x100);
  
  // Test alignment by large number, by full size, and by half size
  assert(aligner.Align(address, 0x1000, 1));
  assert(address == 0);
  assert(!aligner.Align(address, 0x100, 1));
  assert(aligner.Align(address, 0x80, 0x80));
  assert(address == 0x80);
  aligner.Dealloc(address, 0x80);
  aligner.Dealloc(0, 1);
  
  // Test alignment where the first region is unavailable
  assert(aligner.Alloc(address, 0x80));
  assert(address == 0);
  aligner.Dealloc(1, 0x7e);
  // 0x7f and 1 are still reserved
  assert(aligner.Align(address, 0x10, 0x70));
  assert(address == 0x80);
  assert(aligner.Alloc(address, 0x70));
  assert(address == 0x1);
  
  return 0;
}
