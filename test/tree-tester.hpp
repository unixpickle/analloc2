#include "../src/path.hpp"
#include "../src/tree.hpp"

namespace ANAlloc {

class TreeTester {
public:
  TreeTester(Tree & _tree, const char * _name);
  
  void FreeAll();
  void FreeAll(Path p);
  void AllocAll();
  void TestAllocAll();
  void TestSetGet();
  void TestFindFree();
  
private:
  const char * name;
  Tree & tree;
};

}
