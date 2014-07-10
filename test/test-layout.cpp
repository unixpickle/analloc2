#include "scoped-pass.hpp"
#include "../src/topology/layout.hpp"
#include "../src/topology/fixed-desc-list.hpp"
#include "../src/topology/fixed-region-list.hpp"

using namespace ANAlloc;

void TestBasicLayout();
void TestMinAlignmentLayout();
void TestAlignCascade();
void TestUnfitting();
void TestReverseOrder();

int main() {
  TestBasicLayout();
  TestMinAlignmentLayout();
  TestAlignCascade();
  TestUnfitting();
  TestReverseOrder();
  return 0;
}

void TestBasicLayout() {
  ScopedPass("Layout::Run() [basic]");
  
  FixedRegionList<2> regions;
  regions.Push(Region(0, 0x1000));
  regions.Push(Region(0x1000, 0x3000));
  
  FixedDescList<3> descs;
  
  Layout layout(descs, regions, 4, 0x1000, 0x1000);
  layout.Run();
  
  assert(descs.GetCount() == 3);
  assert(descs[0].GetStart() == 0x1000);
  assert(descs[0].GetDepth() == 10); // 0x10 * 0x200 = 0x2000
  assert(descs[1].GetStart() == 0);
  assert(descs[1].GetDepth() == 9); // 0x10 * 0x100 = 0x1000
  assert(descs[2].GetStart() == 0x3000);
  assert(descs[2].GetDepth() == 9); // 0x10 * 0x100 = 0x1000
}

void TestMinAlignmentLayout() {
  ScopedPass("Layout::Run() [min-align]");
  
  FixedRegionList<4> regions;
  regions.Push(Region(0, 0x1000));
  regions.Push(Region(0x1000, 0x2000));
  regions.Push(Region(0x3400, 0xc00));
  regions.Push(Region(0x4000, 0x100));
  
  FixedDescList<5> descriptions;
  
  // first, make sure we handle *no* overflow correctly
  Layout layout1(descriptions, regions, 8, 0x1000, 0x100);
  layout1.Run();
  assert(descriptions.GetCount() == 5);
  assert(descriptions[0].GetStart() == 0x1000);
  assert(descriptions[0].GetDepth() == 6);
  assert(descriptions[1].GetStart() == 0);
  assert(descriptions[1].GetDepth() == 5);
  assert(descriptions[2].GetStart() == 0x4000);
  assert(descriptions[2].GetDepth() == 1);
  assert(descriptions[3].GetStart() == 0x3800);
  assert(descriptions[3].GetDepth() == 4);
  assert(descriptions[4].GetStart() == 0x3400);
  assert(descriptions[4].GetDepth() == 3);
  
  descriptions.Empty();
  Layout layout2(descriptions, regions, 8, 0x1000, 0x800);
  layout2.Run();
  assert(descriptions.GetCount() == 4);
  assert(descriptions[0].GetStart() == 0x1000);
  assert(descriptions[0].GetDepth() == 6);
  assert(descriptions[1].GetStart() == 0);
  assert(descriptions[1].GetDepth() == 5);
  assert(descriptions[2].GetStart() == 0x4000);
  assert(descriptions[2].GetDepth() == 1);
  assert(descriptions[3].GetStart() == 0x3800);
  assert(descriptions[3].GetDepth() == 4);
}

void TestAlignCascade() {
  ScopedPass pass("Layout::Run() [align cascade]");
  
  FixedRegionList<1> regions;
  regions.Push(Region(1, 7));
  
  FixedDescList<4> descs; // an extra Desc, just for kicks
  
  Layout layout1(descs, regions, 0, 4, 1);
  layout1.Run();
  
  assert(descs.GetCount() == 3);
  assert(descs[0].GetStart() == 4);
  assert(descs[0].GetDepth() == 3);
  assert(descs[1].GetStart() == 2);
  assert(descs[1].GetDepth() == 2);
  assert(descs[2].GetStart() == 1);
  assert(descs[2].GetDepth() == 1);
}

void TestUnfitting() {
  ScopedPass pass("Layout::Run() [unfitting]");
  
  FixedRegionList<1> regions;
  regions.Push(Region(1, 6));
  
  FixedDescList<4> descs; // an extra 3 Descs for kicks
  
  Layout layout1(descs, regions, 2, 8, 4);
  layout1.Run();
  assert(descs.GetCount() == 0);
  
  Layout layout2(descs, regions, 2, 4, 1);
  layout2.Run();
  assert(descs.GetCount() == 1);
  assert(descs[0].GetStart() == 2);
  assert(descs[0].GetDepth() == 1);
}

void TestReverseOrder() {
  ScopedPass pass("Layout::Run() [reversed]");
  
  FixedRegionList<2> regions;
  regions.Push(Region(0, 4));
  regions.Push(Region(5, 4));
  
  FixedDescList<4> descs; // an extra 2 Descs for kicks
  
  Layout layout(descs, regions, 2, 4, 1);
  layout.Run();
  
  assert(descs.GetCount() == 2);
  assert(descs[0].GetStart() == 0);
  assert(descs[0].GetDepth() == 1);
  assert(descs[1].GetStart() == 5);
  assert(descs[1].GetDepth() == 1);
}
