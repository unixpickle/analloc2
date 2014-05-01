#include "../src/tree.h"
#include <iostream>
#include <cassert>

using namespace std;

void TestPathParent();
void TestSiblings();
void TestCoords();

int main() {
  TestPathParent();
  TestSiblings();
  TestCoords();
  return 0;
}

void TestPathParent() {
  cout << "testing PathParent() ... ";
  assert(ANAlloc::PathParent(0) == 0);
  
  assert(ANAlloc::PathParent(1) == 0);
  assert(ANAlloc::PathParent(2) == 0);
  
  assert(ANAlloc::PathParent(3) == 1);
  assert(ANAlloc::PathParent(4) == 1);
  assert(ANAlloc::PathParent(5) == 2);
  assert(ANAlloc::PathParent(6) == 2);
  
  assert(ANAlloc::PathParent(7) == 3);
  assert(ANAlloc::PathParent(8) == 3);
  assert(ANAlloc::PathParent(9) == 4);
  assert(ANAlloc::PathParent(10) == 4);
  assert(ANAlloc::PathParent(11) == 5);
  assert(ANAlloc::PathParent(12) == 5);
  assert(ANAlloc::PathParent(13) == 6);
  assert(ANAlloc::PathParent(14) == 6);
  cout << "passed!" << endl;
}

void TestSiblings() {
  cout << "testing Path[Left/Right/Sibling]() ... ";
  
  assert(ANAlloc::PathLeft(0) == 1);
  assert(ANAlloc::PathRight(0) == 2);
  
  assert(ANAlloc::PathLeft(1) == 3);
  assert(ANAlloc::PathRight(1) == 4);
  assert(ANAlloc::PathLeft(2) == 5);
  assert(ANAlloc::PathRight(2) == 6);
  
  assert(ANAlloc::PathSibling(0) == 0);
  assert(ANAlloc::PathSibling(1) == 2);
  assert(ANAlloc::PathSibling(2) == 1);
  assert(ANAlloc::PathSibling(3) == 4);
  assert(ANAlloc::PathSibling(4) == 3);
  
  cout << "passed!" << endl;
}

void TestCoords() {
  cout << "testing Path[Index/Depth]() ... ";
  
  assert(ANAlloc::PathDepth(0) == 0);
  assert(ANAlloc::PathDepth(1) == 1);
  assert(ANAlloc::PathDepth(2) == 1);
  assert(ANAlloc::PathDepth(3) == 2);
  assert(ANAlloc::PathDepth(4) == 2);
  assert(ANAlloc::PathDepth(5) == 2);
  assert(ANAlloc::PathDepth(6) == 2);
  assert(ANAlloc::PathDepth(7) == 3);
  assert(ANAlloc::PathDepth(8) == 3);
  assert(ANAlloc::PathDepth(9) == 3);
  assert(ANAlloc::PathDepth(10) == 3);
  assert(ANAlloc::PathDepth(11) == 3);
  assert(ANAlloc::PathDepth(12) == 3);
  assert(ANAlloc::PathDepth(13) == 3);
  assert(ANAlloc::PathDepth(14) == 3);
  
  assert(ANAlloc::PathIndex(0) == 0);
  assert(ANAlloc::PathIndex(1) == 0);
  assert(ANAlloc::PathIndex(2) == 1);
  assert(ANAlloc::PathIndex(3) == 0);
  assert(ANAlloc::PathIndex(4) == 1);
  assert(ANAlloc::PathIndex(5) == 2);
  assert(ANAlloc::PathIndex(6) == 3);
  
  cout << "passed!" << endl;
}
