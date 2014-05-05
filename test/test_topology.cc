#include <iostream>
#include "../src/topology.h"
#include "../src/btree.h"
#include "../src/bbtree.h"

using namespace std;

void TestBasicLayout();
void TestMinAlignmentLayout();

template <class T>
void TestBasicReserve(string name);

template <class T>
void TestOverlappingReserve(string name);

template <class T>
void TestPointerConversion(string name);

int main() {
  TestBasicLayout();
  TestMinAlignmentLayout();
  TestBasicReserve<ANAlloc::BTree>("BTree");
  TestBasicReserve<ANAlloc::BBTree>("BBTree");
  TestOverlappingReserve<ANAlloc::BTree>("BTree");
  TestOverlappingReserve<ANAlloc::BBTree>("BBTree");
  TestPointerConversion<ANAlloc::BTree>("BTree");
  TestPointerConversion<ANAlloc::BBTree>("BBTree");
  
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

template <class T>
void TestBasicReserve(string name) {
  cout << "testing AllocatorList<" << name << ">::Reserve() [basic] ... ";
  
  ANAlloc::Region reg1(0, 0x1000);
  ANAlloc::Region reg2(0x1000, 0x3000);
  
  ANAlloc::Region memoryRegions[2];
  memoryRegions[0] = reg1;
  memoryRegions[1] = reg2;
  
  ANAlloc::AllocatorList<3, T> list(0x1000, 0x1000, 0x10,
                                    memoryRegions, 2);
  list.GenerateDescriptions();
  size_t size = list.BitmapByteCount();
  uint8_t * buffer = new uint8_t[size];
  list.GenerateAllocators(buffer);
  
  list.Reserve(reg1);
  
  assert(list.GetTrees()[0].GetType(0) == T::NodeTypeFree);
  assert(list.GetTrees()[1].GetType(0) == T::NodeTypeData);
  assert(list.GetTrees()[2].GetType(0) == T::NodeTypeFree);
  
  list.Reserve(reg2);
  assert(list.GetTrees()[0].GetType(0) == T::NodeTypeData);
  assert(list.GetTrees()[1].GetType(0) == T::NodeTypeData);
  assert(list.GetTrees()[2].GetType(0) == T::NodeTypeData);
  
  delete buffer;
  cout << "passed!" << endl;
}

template <class T>
void TestOverlappingReserve(string name) {
  cout << "testing AllocatorList<" << name
    << ">::Reserve() [overlapping] ... ";
  
  ANAlloc::Region reg1(0, 0x1000);
  ANAlloc::Region reg2(0x1000, 0x1000);
  ANAlloc::Region reg3(0x2000, 0x1000);
  
  ANAlloc::Region memoryRegions[3];
  memoryRegions[0] = reg1;
  memoryRegions[1] = reg2;
  memoryRegions[2] = reg3;
  
  ANAlloc::AllocatorList<3, T> list(0x1000, 0x1000, 0x10,
                                    memoryRegions, 3);
  list.GenerateDescriptions();
  size_t size = list.BitmapByteCount();
  uint8_t * buffer = new uint8_t[size];
  list.GenerateAllocators(buffer);
  
  ANAlloc::Region reg(0x10, 0x2000);
  list.Reserve(reg);
  
  // verify structure of tree 0
  ANAlloc::Path p = 0;
  T & tree0 = list.GetTrees()[0];
  for (int i = 0; i < tree0.Depth() - 1; i++) {
    ANAlloc::Path left = ANAlloc::PathLeft(p);
    ANAlloc::Path right = left + 1;
    assert(tree0.GetType(p) == T::NodeTypeContainer);
    assert(tree0.GetType(right) == T::NodeTypeData);
    if (i + 2 == tree0.Depth()) {
      assert(tree0.GetType(left) == T::NodeTypeFree);
    }
    p = left;
  }
  
  // verify structure of tree 1
  assert(list.GetTrees()[1].GetType(0) == T::NodeTypeData);
  
  // verify structure of tree 2
  p = 0;
  T & tree2 = list.GetTrees()[2];
  for (int i = 0; i < tree2.Depth() - 1; i++) {
    ANAlloc::Path left = ANAlloc::PathLeft(p);
    ANAlloc::Path right = left + 1;
        
    assert(tree2.GetType(p) == T::NodeTypeContainer);
    assert(tree2.GetType(right) == T::NodeTypeFree);
    if (i + 2 == tree2.Depth()) {
      assert(tree2.GetType(left) == T::NodeTypeData);
    }
    p = left;
  }
  
  delete buffer;
  cout << "passed!" << endl;
}

template <class T>
void TestPointerConversion(string name) {
  cout << "testing AllocatorList<" << name
    << ">::[Pointer/Path]for[Path/Pointer]() ... ";
  
  ANAlloc::Region reg1(0, 0x1000);
  // throw in a gap, just for fun
  ANAlloc::Region reg2(0x1800, 0x1000);
  
  ANAlloc::Region memoryRegions[2];
  memoryRegions[0] = reg1;
  memoryRegions[1] = reg2;
  
  ANAlloc::AllocatorList<2, T> list(0x800, 0x800, 0x10,
                                    memoryRegions, 2);
  list.GenerateDescriptions();
  size_t size = list.BitmapByteCount();
  uint8_t * buffer = new uint8_t[size];
  list.GenerateAllocators(buffer);
  
  // test translation for allocator 0
  ANAlloc::Path p = 0;
  ANAlloc::Allocator<T> & alloc = list.GetAllocators()[0];
  alloc.Alloc(1, p);
  assert(p == 1);
  alloc.Alloc(2, p);
  assert(p == 5);
  assert(list.PointerForPath(0, 1) == 0);
  assert(list.PointerForPath(0, 5) == 0x800);
  int i;
  assert(list.PathForPointer(0, p, i));
  assert(i == 0);
  assert(p == 1);
  assert(list.PathForPointer(0x800, p, i));
  assert(i == 0);
  assert(p == 5);
  assert(!list.PathForPointer(0xc00, p, i));
  
  // test translation for allocator 1
  alloc = list.GetAllocators()[1];
  alloc.Alloc(1, p);
  assert(p == 1);
  alloc.Alloc(2, p);
  assert(p == 5);
  assert(list.PointerForPath(1, 1) == 0x1800);
  assert(list.PointerForPath(1, 5) == 0x2000);
  assert(list.PathForPointer(0x1800, p, i));
  assert(i == 1);
  assert(p == 1);
  assert(list.PathForPointer(0x2000, p, i));
  assert(i == 1);
  assert(p == 5);
  assert(!list.PathForPointer(0x2400, p, i));

  delete buffer;
  cout << "passed!" << endl;
}
