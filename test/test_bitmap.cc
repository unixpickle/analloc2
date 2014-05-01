#include "../src/bitmap.h"
#include <stdio.h>
#include <new>
#include <iostream>

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
  
  cout << "test passed!" << endl;
  
  return 0;
}
