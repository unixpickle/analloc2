#include <iostream>
#include "../src/topology/allocator-list.hpp"
#include "../src/btree.hpp"
#include "../src/bbtree.hpp"

using namespace std;

void TestBasicLayout();
void TestMinAlignmentLayout();
void TestSortedLayout();

template <class T>
void TestBasicReserve(string name);

template <class T>
void TestOverlappingReserve(string name);

template <class T>
void TestPointerConversion(string name);

template <class T>
void TestAllocFree(string name);

template <class T>
void TestAllocFreeBad(string name);

template <class T>
void TestAvailableSize(string name);

template <class T>
void TestAllocHigh(string name);

template <class T>
void TestAllocLow(string name);

int main() {
  TestBasicLayout();
  TestMinAlignmentLayout();
  TestSortedLayout();
  TestBasicReserve<ANAlloc::BTree>("BTree");
  TestBasicReserve<ANAlloc::BBTree>("BBTree");
  TestOverlappingReserve<ANAlloc::BTree>("BTree");
  TestOverlappingReserve<ANAlloc::BBTree>("BBTree");
  TestPointerConversion<ANAlloc::BTree>("BTree");
  TestPointerConversion<ANAlloc::BBTree>("BBTree");
  TestAllocFree<ANAlloc::BTree>("BTree");
  TestAllocFree<ANAlloc::BBTree>("BBTree");
  TestAllocFreeBad<ANAlloc::BTree>("BTree");
  TestAllocFreeBad<ANAlloc::BBTree>("BBTree");
  TestAvailableSize<ANAlloc::BTree>("BTree");
  TestAvailableSize<ANAlloc::BBTree>("BBTree");
  TestAllocHigh<ANAlloc::BTree>("BTree");
  TestAllocHigh<ANAlloc::BBTree>("BBTree");
  TestAllocLow<ANAlloc::BTree>("BTree");
  TestAllocLow<ANAlloc::BBTree>("BBTree");
  
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
  assert(list.GetDescriptions()[0].GetStart() == 0x1000);
  assert(list.GetDescriptions()[0].GetDepth() == 10);
  assert(list.GetDescriptions()[1].GetStart() == 0);
  assert(list.GetDescriptions()[1].GetDepth() == 9);
  assert(list.GetDescriptions()[2].GetStart() == 0x3000);
  assert(list.GetDescriptions()[2].GetDepth() == 9);
  
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
  assert(list.GetDescriptions()[0].GetStart() == 0x1000);
  assert(list.GetDescriptions()[0].GetDepth() == 6);
  assert(list.GetDescriptions()[1].GetStart() == 0);
  assert(list.GetDescriptions()[1].GetDepth() == 5);
  assert(list.GetDescriptions()[2].GetStart() == 0x4000);
  assert(list.GetDescriptions()[2].GetDepth() == 1);
  assert(list.GetDescriptions()[3].GetStart() == 0x3800);
  assert(list.GetDescriptions()[3].GetDepth() == 4);
  assert(list.GetDescriptions()[4].GetStart() == 0x3400);
  assert(list.GetDescriptions()[4].GetDepth() == 3);
  
  // now, raise the minimum alignment to 0x800
  ANAlloc::AllocatorList<5, ANAlloc::BBTree> list2(0x1000, 0x800, 0x100,
                                                   regions, 4);
  list2.GenerateDescriptions();
  assert(list2.GetDescriptionCount() == 4);
  assert(list2.GetDescriptions()[0].GetStart() == 0x1000);
  assert(list2.GetDescriptions()[0].GetDepth() == 6);
  assert(list2.GetDescriptions()[1].GetStart() == 0);
  assert(list2.GetDescriptions()[1].GetDepth() == 5);
  assert(list2.GetDescriptions()[2].GetStart() == 0x4000);
  assert(list2.GetDescriptions()[2].GetDepth() == 1);
  assert(list2.GetDescriptions()[3].GetStart() == 0x3800);
  assert(list2.GetDescriptions()[3].GetDepth() == 4);
  
  cout << "passed!" << endl;
}

void TestSortedLayout() {
  cout << "testing AllocatorList::GenerateDescriptions(true) ... ";
  
  // when we go down in alignment, the order will get scrambled like eggs
  ANAlloc::Region reg1(0, 0x1000);
  ANAlloc::Region reg2(0x1800, 0x1800);
  
  ANAlloc::Region regions[2];
  regions[0] = reg1;
  regions[1] = reg2;
  
  ANAlloc::AllocatorList<4, ANAlloc::BBTree> unsorted(0x1000, 0x800, 0x100,
                                                      regions, 2);
  unsorted.GenerateDescriptions();
  assert(unsorted.GetDescriptionCount() == 3);
  assert(unsorted.GetDescriptions()[0].GetStart() == 0x0);
  assert(unsorted.GetDescriptions()[0].GetDepth() == 5);
  assert(unsorted.GetDescriptions()[1].GetStart() == 0x2000);
  assert(unsorted.GetDescriptions()[1].GetDepth() == 5);
  assert(unsorted.GetDescriptions()[2].GetStart() == 0x1800);
  assert(unsorted.GetDescriptions()[2].GetDepth() == 4);
  
  ANAlloc::AllocatorList<4, ANAlloc::BBTree> sorted(0x1000, 0x800, 0x100,
                                                    regions, 2);
  sorted.GenerateDescriptions(true);
  assert(sorted.GetDescriptionCount() == 3);
  assert(sorted.GetDescriptions()[0].GetStart() == 0x0);
  assert(sorted.GetDescriptions()[0].GetDepth() == 5);
  assert(sorted.GetDescriptions()[1].GetStart() == 0x1800);
  assert(sorted.GetDescriptions()[1].GetDepth() == 4);
  assert(sorted.GetDescriptions()[2].GetStart() == 0x2000);
  assert(sorted.GetDescriptions()[2].GetDepth() == 5);
  
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
  
  list.Reserve(0x1000);
  
  assert(list.GetTrees()[0].GetType(0) == T::NodeTypeFree);
  assert(list.GetTrees()[1].GetType(0) == T::NodeTypeData);
  assert(list.GetTrees()[2].GetType(0) == T::NodeTypeFree);
  
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
  
  list.Reserve(0x200f); // offset by 1 to make sure it rounds up right
  
  // verify structure of tree 0
  assert(list.GetTrees()[0].GetType(0) == T::NodeTypeData);
  
  // verify structure of tree 1
  assert(list.GetTrees()[1].GetType(0) == T::NodeTypeData);
  
  // verify structure of tree 2
  ANAlloc::Path p = 0;
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

template <class T>
void TestAllocFree(string name) {
  cout << "testing AllocatorList<" << name
    << ">::[Alloc/Free]Pointer() [basic] ... ";
  
  size_t sizeOut = 0;
  
  ANAlloc::Region reg1(0, 0x1000);
  ANAlloc::Region reg2(0x1000, 0x1000);
  ANAlloc::Region reg3(0x2800, 0x1000); // smaller alignment
  
  ANAlloc::Region memoryRegions[3];
  memoryRegions[0] = reg1;
  memoryRegions[1] = reg2;
  memoryRegions[2] = reg3;
  
  ANAlloc::AllocatorList<4, T> list(0x1000, 0x800, 0x10,
                                    memoryRegions, 3);
  list.GenerateDescriptions();
  size_t size = list.BitmapByteCount();
  uint8_t * buffer = new uint8_t[size];
  list.GenerateAllocators(buffer);
  
  assert(list.GetDescriptionCount() == 4);
  assert(list.GetDescriptions()[0].GetStart() == 0);
  assert(list.GetDescriptions()[0].GetDepth() == 9);
  assert(list.GetDescriptions()[1].GetStart() == 0x1000);
  assert(list.GetDescriptions()[1].GetDepth() == 9);
  assert(list.GetDescriptions()[2].GetStart() == 0x3000);
  assert(list.GetDescriptions()[2].GetDepth() == 8);
  assert(list.GetDescriptions()[3].GetStart() == 0x2800);
  assert(list.GetDescriptions()[3].GetDepth() == 8);
  
  // first, go for the easy case of allocating everything
  uintptr_t outPtr;
  assert(list.AllocPointer(0x1000, 0x1000, outPtr, &sizeOut));
  assert(0 == outPtr);
  assert(0x1000 == sizeOut);
  assert(list.GetTrees()[0].GetType(0) == T::NodeTypeData);
  
  assert(list.AllocPointer(0x1000, 0x1000, outPtr, &sizeOut));
  assert(0x1000 == outPtr);
  assert(0x1000 == sizeOut);
  assert(list.GetTrees()[1].GetType(0) == T::NodeTypeData);
  
  assert(!list.AllocPointer(0x1000, 0x800, outPtr, NULL));
  assert(list.AllocPointer(0x800, 0x800, outPtr, &sizeOut));
  assert(0x3000 == outPtr);
  assert(0x800 == sizeOut);
  assert(list.GetTrees()[2].GetType(0) == T::NodeTypeData);
  
  assert(list.AllocPointer(0x800, 0x800, outPtr, &sizeOut));
  assert(0x2800 == outPtr);
  assert(0x800 == sizeOut);
  assert(list.GetTrees()[3].GetType(0) == T::NodeTypeData);
  
  // make sure freeing these addresses works
  list.FreePointer(0x2800);
  assert(list.GetTrees()[3].GetType(0) == T::NodeTypeFree);
  
  // attempt to allocate a smaller aligned region
  assert(list.AllocPointer(0x400, 0x400, outPtr, &sizeOut));
  assert(0x2800 == outPtr);
  assert(0x400 == sizeOut);
  
  assert(list.GetTrees()[3].GetType(0) == T::NodeTypeContainer);
  assert(list.GetTrees()[3].GetType(1) == T::NodeTypeData);
  assert(list.GetTrees()[3].GetType(2) == T::NodeTypeFree);
  
  assert(!list.AllocPointer(0x400, 0x800, outPtr, NULL));
  assert(list.AllocPointer(0x400, 0x400, outPtr, NULL));
  assert(0x2c00 == outPtr);
  assert(list.GetTrees()[3].GetType(2) == T::NodeTypeData);
  
  list.FreePointer(0x2800);
  assert(list.GetTrees()[3].GetType(1) == T::NodeTypeFree);
  assert(list.GetTrees()[3].GetType(2) == T::NodeTypeData);
  list.FreePointer(0x2c00);
  assert(list.GetTrees()[3].GetType(0) == T::NodeTypeFree);
  
  list.FreePointer(0x3000);
  assert(list.GetTrees()[2].GetType(0) == T::NodeTypeFree);
  list.FreePointer(0x1000);
  assert(list.GetTrees()[1].GetType(0) == T::NodeTypeFree);
  list.FreePointer(0);
  assert(list.GetTrees()[0].GetType(0) == T::NodeTypeFree);
  
  // try nesting this (fun chiasmus)
  assert(list.AllocPointer(0x10, 0x10, outPtr, &sizeOut));
  assert(0 == outPtr);
  assert(0x10 == sizeOut);
  assert(list.AllocPointer(0x20, 0x20, outPtr, &sizeOut));
  assert(0x20 == outPtr);
  assert(0x20 == sizeOut);
  assert(list.AllocPointer(0x10, 0x10, outPtr, &sizeOut));
  assert(0x10 == outPtr);
  assert(0x10 == sizeOut);
  
  for (int i = 8; i > 1; i--) {
    size_t size = (0x10L << i);
    assert(list.AllocPointer(size, size, outPtr, &sizeOut));
    assert(size == sizeOut);
    assert(size == outPtr);
  }
  for (int i = 8; i > 1; i--) {
    list.FreePointer((0x10L << i));
  }
  
  list.FreePointer(0x0);
  list.FreePointer(0x10);
  list.FreePointer(0x20);
  
  cout << "passed!" << endl;
  delete buffer;
}

template <class T>
void TestAllocFreeBad(string name) {
  cout << "testing AllocatorList<" << name
    << ">::[Alloc/Free]Pointer() [bad] ... ";
  
  size_t sizeOut = 0;
  
  ANAlloc::Region reg1(1, 0x1000);
  
  ANAlloc::AllocatorList<2, T> list(1, 1, 0x10,
                                    &reg1, 1);
  list.GenerateDescriptions();
  size_t size = list.BitmapByteCount();
  uint8_t * buffer = new uint8_t[size];
  list.GenerateAllocators(buffer);
  
  assert(list.GetDescriptionCount() == 1);
  assert(list.GetDescriptions()[0].GetStart() == 1);
  assert(list.GetDescriptions()[0].GetDepth() == 9);
  
  uintptr_t outPtr;
  assert(list.AllocPointer(0x10, 0x10, outPtr, &sizeOut));
  assert(0x10 == outPtr);
  assert(0x11 == sizeOut);
  
  list.FreePointer(0x10);
  assert(list.GetTrees()[0].GetType(0) == T::NodeTypeFree);
  
  cout << "passed!" << endl;
  delete buffer;
}

template <class T>
void TestAvailableSize(string name) {
  cout << "testing AllocatorList<" << name
    << ">::[Alloc/Free]Pointer() [bad] ... ";
  
  size_t sizeOut = 0;
  
  ANAlloc::Region reg1(1, 0x10000);
  
  ANAlloc::AllocatorList<2, T> list(1, 1, 0x10,
                                    &reg1, 1);
  list.GenerateDescriptions();
  size_t size = list.BitmapByteCount();
  uint8_t * buffer = new uint8_t[size];
  list.GenerateAllocators(buffer);
  
  list.Reserve(0x200);
  
  assert(list.GetAvailableSpace() == 0xfe00);
  
  uintptr_t outPtr;
  list.AllocPointer(1, 1, outPtr, NULL);
  assert(list.GetAvailableSpace() == 0xfdf0);
  
  list.FreePointer(outPtr);
  assert(list.GetAvailableSpace() == 0xfe00);
  
  list.AllocPointer(1, 0x10, outPtr, NULL);
  assert(list.GetAvailableSpace() == 0xfde0);
  
  list.FreePointer(outPtr);
  assert(list.GetAvailableSpace() == 0xfe00);
  
  cout << "passed!" << endl;
  delete buffer;
}

template <class T>
void TestAllocHigh(string name) {
  cout << "testing AllocatorList<" << name << ">::AllocDescending() ... ";
  
  ANAlloc::Region reg1(0, 0x1000);
  ANAlloc::Region reg2(0x1000, 0x1000);
  ANAlloc::Region regions[2];
  regions[0] = reg1;
  regions[1] = reg2;
  
  ANAlloc::AllocatorList<3, T> list(0x1000, 0x1000, 0x10,
                                    regions, 2);
  list.GenerateDescriptions(true);
  assert(list.GetDescriptionCount() == 2);
  
  uint8_t * buffer = new uint8_t[list.BitmapByteCount()];
  list.GenerateAllocators(buffer);
  
  uintptr_t ptr;
  size_t size;
  assert(list.AllocDescending(0x10, 1, ptr, &size));
  assert(size == 0x10);
  assert(ptr == 0x1000);
  list.FreePointer(ptr);
  assert(list.GetTrees()[0].GetType(0) == T::NodeTypeFree);
  assert(list.GetTrees()[1].GetType(0) == T::NodeTypeFree);
  assert(!list.AllocDescending(1, 1, ptr, &size, 0x1001));
  assert(list.AllocDescending(1, 1, ptr, &size, 0x1000));
  assert(size == 0x10);
  assert(ptr == 0x1000);
  
  assert(!list.AllocDescending(0x1000, 1, ptr, &size, 1));
  assert(list.AllocDescending(0x1000, 1, ptr, &size, 0));
  assert(!ptr);
  assert(size == 0x1000);
  
  cout << "passed!" << endl;
  delete buffer;
}

template <class T>
void TestAllocLow(string name) {
  cout << "testing AllocatorList<" << name << ">::AllocAscending() ... ";
  
  ANAlloc::Region reg1(0, 0x1000);
  ANAlloc::Region reg2(0x1000, 0x1000);
  ANAlloc::Region regions[2];
  regions[0] = reg1;
  regions[1] = reg2;
  
  ANAlloc::AllocatorList<3, T> list(0x1000, 0x1000, 0x10,
                                    regions, 2);
  list.GenerateDescriptions(true);
  assert(list.GetDescriptionCount() == 2);
  
  uint8_t * buffer = new uint8_t[list.BitmapByteCount()];
  list.GenerateAllocators(buffer);
  
  uintptr_t ptr;
  size_t size;
  assert(list.AllocAscending(0x10, 1, ptr, &size));
  assert(size == 0x10);
  assert(ptr == 0);
  list.FreePointer(ptr);
  assert(list.GetTrees()[0].GetType(0) == T::NodeTypeFree);
  assert(list.GetTrees()[1].GetType(0) == T::NodeTypeFree);
  assert(!list.AllocAscending(1, 1, ptr, &size, 0xffe));
  assert(list.AllocAscending(1, 1, ptr, &size, 0xfff));
  assert(size == 0x10);
  assert(ptr == 0);
  
  assert(!list.AllocAscending(0x1000, 1, ptr, &size, 0x1ffe));
  assert(list.AllocAscending(0x1000, 1, ptr, &size, 0x1fff));
  assert(ptr = 0x1000);
  assert(size == 0x1000);
  
  cout << "passed!" << endl;
  delete buffer;
}
