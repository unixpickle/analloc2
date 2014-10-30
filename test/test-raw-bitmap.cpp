#include "scoped-pass.hpp"
#include <analloc2/bitmap>
#include <ansa/numeric-info>
#include <ansa/cstring>
#include <cstdint>
#include <cassert>

using namespace ansa;
using namespace analloc;

template <typename T>
void TestSetBit();

template <typename T>
void TestSetBits();

int main() {
  TestSetBit<unsigned char>();
  TestSetBit<unsigned short>();
  TestSetBit<unsigned int>();
  TestSetBit<unsigned long>();
  TestSetBit<unsigned long long>();
  
  TestSetBits<unsigned char>();
  TestSetBits<unsigned short>();
  TestSetBits<unsigned int>();
  TestSetBits<unsigned long>();
  TestSetBits<unsigned long long>();
  return 0;
}

template <typename T>
void TestSetBit() {
  ScopedPass pass("RawBitmap<", NumericInfo<T>::name, ">::[Set/Get]Bit()");
  
  T units[0x10];
  ansa::Bzero((void *)units, sizeof(units));
  size_t bitCount = sizeof(T) * 0x80;
  RawBitmap<T> bm(units, bitCount);
  for (size_t i = 0; i < bitCount; ++i) {
    assert(!bm.GetBit(i));
    bm.SetBit(i, true);
  }
  for (size_t i = 0; i < bitCount; ++i) {
    assert(bm.GetBit(i));
    bm.SetBit(i, false);
    assert(!bm.GetBit(i));
    for (size_t j = 0; j < bitCount; ++j) {
      if (j == i) continue;
      assert(bm.GetBit(j));
    }
    bm.SetBit(i, true);
    assert(bm.GetBit(i));
  }
}

template <typename T>
void TestSetBits() {
  ScopedPass pass("RawBitmap<", NumericInfo<T>::name, ">::[Set/Get]Bits()");
  
  T units[0x10];
  ansa::Bzero((void *)units, sizeof(units));
  size_t bitCount = sizeof(T) * 0x80;
  RawBitmap<T> bm(units, bitCount);
  // set a bunch of multibit sequences
  for (size_t i = 0; i < bitCount - 5; ++i) {
    bm.template SetBits<uint8_t>(i, 5, 0x15);
    assert(bm.template GetBits<uint8_t>(i, 5) == 0x15);
    assert(bm.GetBit(i) == true);
    assert(bm.GetBit(i + 1) == false);
    assert(bm.GetBit(i + 2) == true);
    assert(bm.GetBit(i + 3) == false);
    assert(bm.GetBit(i + 4) == true);
    bm.template SetBits<uint8_t>(i, 5, 3);
    assert(bm.template GetBits<uint8_t>(i, 5) == 3);
    assert(bm.GetBit(i) == false);
    assert(bm.GetBit(i + 1) == false);
    assert(bm.GetBit(i + 2) == false);
    assert(bm.GetBit(i + 3) == true);
    assert(bm.GetBit(i + 4) == true);
    bm.template SetBits<uint8_t>(i, 5, 0);
    assert(bm.template GetBits<uint8_t>(i, 5) == 0);
    for (size_t j = i; j < i + 5; ++j) {
      assert(bm.GetBit(j) == false);
    }
  }
}
