#include <iostream>
#include "../src/topology.h"
#include "../src/btree.h"
#include "../src/bbtree.h"

void TestBasicLayout();
void TestOverflowLayout();

int main() {
  TestBasicLayout();
  TestOverflowLayout();
  
  return 0;
}

void TestBasicLayout() {
  std::cout << "testing AllocatorList::GenerateDescriptions() [basic] ... ";
  
  ANAlloc::Region reg1(0, 0x1000);
  ANAlloc::Region reg2(0x1000, 0x3000);
  
  ANAlloc::Region memoryRegions[2];
  memoryRegions[0] = reg1;
  memoryRegions[1] = reg2;
  
  ANAlloc::AllocatorList<4, ANAlloc::BBTree> list(0x1000, 0x1000, 0x10,
                                                  memoryRegions, 2);
  list.GenerateDescriptions();
  
  assert(list.GetDescriptionCount() == 3);
  assert(list.GetDescriptions()[0].start == 0x1000);
  assert(list.GetDescriptions()[0].depth == 10);
  assert(list.GetDescriptions()[1].start == 0);
  assert(list.GetDescriptions()[1].depth == 9);
  assert(list.GetDescriptions()[2].start == 0x3000);
  assert(list.GetDescriptions()[2].depth == 9);
  
  std::cout << "passed!" << std::endl;
}

void TestOverflowLayout() {
  
}
