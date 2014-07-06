#include <iostream>
#include <cassert>
#include "../src/tree.hpp"

using namespace ANAlloc;

void TestComparison();
void TestPathParent();
void TestSiblings();
void TestTreeIndex();

int main() {
  TestComparison();
  TestPathParent();
  TestSiblings();
  TestTreeIndex();
  return 0;
}

void TestComparison() {
  std::cout << "testing Path::operator==() ... " << std::flush;
  
  assert(Path(0, 0) == Path(0, 0));
  assert(Path(1, 0) != Path(0, 0));
  assert(Path(0, 1) != Path(0, 0));
  
  std::cout << "passed!" << std::endl;
}

void TestPathParent() {
  std::cout << "testing Path::Parent() ... " << std::flush;
  assert(Path(1, 0).Parent() == Path(0, 0));
  assert(Path(1, 1).Parent() == Path(0, 0));
  
  assert(Path(2, 0).Parent() == Path(1, 0));
  assert(Path(2, 1).Parent() == Path(1, 0));
  assert(Path(2, 2).Parent() == Path(1, 1));
  assert(Path(2, 3).Parent() == Path(1, 1));
  
  std::cout << "passed!" << std::endl;
}

void TestSiblings() {
  std::cout << "testing Path::[Left/Right/Sibling]() ... " << std::flush;
  
  assert(Path(0, 0).Left() == Path(1, 0));
  assert(Path(0, 0).Right() == Path(1, 1));
  
  assert(Path(1, 0).Left() == Path(2, 0));
  assert(Path(1, 0).Right() == Path(2, 1));
  assert(Path(1, 1).Left() == Path(2, 2));
  assert(Path(1, 1).Right() == Path(2, 3));
  
  std::cout << "passed!" << std::endl;
}

void TestTreeIndex() {
  std::cout << "testing Path::TreeIndex() ... " << std::flush;
  
  assert(Path(0, 0).TreeIndex() == 0);
  
  assert(Path(1, 0).TreeIndex() == 1);
  assert(Path(1, 1).TreeIndex() == 2);
  
  assert(Path(2, 0).TreeIndex() == 3);
  assert(Path(2, 1).TreeIndex() == 4);
  assert(Path(2, 2).TreeIndex() == 5);
  assert(Path(2, 3).TreeIndex() == 6);
  
  assert(Path(3, 0).TreeIndex() == 7);
  assert(Path(3, 1).TreeIndex() == 8);
  assert(Path(3, 2).TreeIndex() == 9);
  assert(Path(3, 3).TreeIndex() == 10);
  assert(Path(3, 4).TreeIndex() == 11);
  assert(Path(3, 5).TreeIndex() == 12);
  assert(Path(3, 6).TreeIndex() == 13);
  assert(Path(3, 7).TreeIndex() == 14);
  
  std::cout << "passed!" << std::endl;
}
