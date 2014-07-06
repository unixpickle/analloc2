#include <new>
#include <cassert>
#include <iostream>
#include "../src/bitmap.hpp"

#define RAND_BIT(i) (bool)((i % 2) ^ (i % 3 != 2) ^ (i % 7 == 0))

using namespace std;

int main() {
  uint8_t * buffer = new uint8_t[0x1000];
  ANAlloc::Bitmap bm(buffer, 0x8000);
  
  for (int i = 0; i < 0x8000; i++) {
    bool theValue = RAND_BIT(i);
    bm.SetBit(i, theValue);
  }
  
  for (int i = 0; i < 0x8000; i++) {
    bool theValue = RAND_BIT(i);
    if (bm.GetBit(i) != theValue) {
      cout << "bit at index" << i << " is invalid." << endl;
      abort();
    }
  }
  
  // test group reads
  bm.SetBit(0, true);
  bm.SetBit(1, false);
  bm.SetBit(2, false);
  bm.SetBit(3, false);
  bm.SetBit(4, true);
  bm.SetBit(5, true);
  bm.SetBit(6, false);
  bm.SetBit(7, true);
  bm.SetBit(8, false);
  bm.SetBit(9, true);
  
  uintptr_t value = bm.GetMultibit(0, 10);
  assert(value == 0b1000110101);
  
  bm.SetMultibit(0, 10, 0b0110011010);
  assert(bm.GetBit(0) == false);
  assert(bm.GetBit(1) == true);
  assert(bm.GetBit(2) == true);
  assert(bm.GetBit(3) == false);
  assert(bm.GetBit(4) == false);
  assert(bm.GetBit(5) == true);
  assert(bm.GetBit(6) == true);
  assert(bm.GetBit(7) == false);
  assert(bm.GetBit(8) == true);
  assert(bm.GetBit(9) == false);
  
  cout << "test passed!" << endl;
  
  return 0;
}
