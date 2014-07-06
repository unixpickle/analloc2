#ifndef __ANALLOC2_TREE_TESTER_HPP__
#define __ANALLOC2_TREE_TESTER_HPP__

#include "../src/path.hpp"
#include "../src/tree.hpp"

namespace ANAlloc {

class TreeTester {
public:
  TreeTester(Tree & _tree, const char * _name);
  
  void AllocAll();
  void TestAllocAll();
  void TestSetGet();
  void TestFindFree();
  
private:
  const char * name;
  Tree & tree;
};

}

#endif
