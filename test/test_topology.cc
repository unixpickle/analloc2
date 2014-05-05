#include <iostream>
#include "../src/topology.h"
#include "../src/btree.h"
#include "../src/bbtree.h"

using namespace std;

void TestBasicLayout();
void TestMinAlignmentLayout();

int main() {
  TestBasicLayout();
  TestMinAlignmentLayout();
  
  return 0;
}

void TestBasicLayout() {
  cout << "testing AllocatorList::GenerateDescriptions() [basic] ... ";
  
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
  
  cout << "passed!" << endl;
}

void TestMinAlignmentLayout() {
  cout << "testing AllocatorList::GenerateDescriptions() [min-align] ... ";
  ANAlloc::Region reg1(0, 0x1000);
  ANAlloc::Region reg2(0x1000, 0x2000);
  ANAlloc::Region reg3(0x3400, 0xc00);
  ANAlloc::Region reg4(0x4000, 0x100);
  
  ANAlloc::Region regions[4];
  regions[0] = reg1;
  regions[1] = reg2;
  regions[2] = reg3;
  regions[3] = reg4;
  
  // first, make sure we handle *no* overflow correctly
  ANAlloc::AllocatorList<5, ANAlloc::BBTree> list(0x1000, 0x100, 0x100,
                                                  regions, 4);
  list.GenerateDescriptions();
  assert(list.GetDescriptionCount() == 5);
  assert(list.GetDescriptions()[0].start == 0x1000);
  assert(list.GetDescriptions()[0].depth == 6);
  assert(list.GetDescriptions()[1].start == 0);
  assert(list.GetDescriptions()[1].depth == 5);
  assert(list.GetDescriptions()[2].start == 0x4000);
  assert(list.GetDescriptions()[2].depth == 1);
  assert(list.GetDescriptions()[3].start == 0x3800);
  assert(list.GetDescriptions()[3].depth == 4);
  assert(list.GetDescriptions()[4].start == 0x3400);
  assert(list.GetDescriptions()[4].depth == 3);
  
  // now, raise the minimum alignment to 0x800
  ANAlloc::AllocatorList<5, ANAlloc::BBTree> list2(0x1000, 0x800, 0x100,
                                                   regions, 4);
  list2.GenerateDescriptions();
  assert(list2.GetDescriptionCount() == 4);
  assert(list2.GetDescriptions()[0].start == 0x1000);
  assert(list2.GetDescriptions()[0].depth == 6);
  assert(list2.GetDescriptions()[1].start == 0);
  assert(list2.GetDescriptions()[1].depth == 5);
  assert(list2.GetDescriptions()[2].start == 0x4000);
  assert(list2.GetDescriptions()[2].depth == 1);
  assert(list2.GetDescriptions()[3].start == 0x3800);
  assert(list2.GetDescriptions()[3].depth == 4);
  
  cout << "passed!" << endl;
}
