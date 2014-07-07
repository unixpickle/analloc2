#ifndef __ANALLOC2_TREE_TESTER_HPP__
#define __ANALLOC2_TREE_TESTER_HPP__

#include "../src/tree/path.hpp"
#include "../src/tree/tree.hpp"

namespace ANAlloc {

class TreeTester {
public:
  TreeTester(Tree & _tree, const char * _name);
  
  void AllocAll();
  void TestAllocAll();
  void TestSetGet();
  void TestFindFree();
  void TestFindAligned();
  
  void TestBaseAlloc();
  void TestFragAlloc();
  void TestExhaustiveAlloc();
  void TestAlignAlloc();
  void TestFindByShadow();
  void TestCarveCenter();
  void TestCarveSide();
  void TestCarveFull();
  
  void TestAll();
  
private:
  const char * name;
  Tree & tree;
};

}

#endif
