#include <string>
#include <iostream>

struct ScopedPass {
  ~ScopedPass() {
    std::cout << "passed!" << std::endl;
  }
};

template <class T>
void FreeAll(T & tree) {
  ANAlloc::Path pathCount = (1 << (tree.Depth() - 1));
  for (ANAlloc::Path i = 0; i < pathCount; i++) {
    if (tree.GetType(pathCount - i - 1) != T::NodeTypeFree) {
      tree.SetType(pathCount - i - 1, T::NodeTypeFree);
    }
  }
}

template <class T>
ANAlloc::Path AllocAll(T & tree) {
  FreeAll(tree);
  ANAlloc::Path p = 0;
  for (int i = 0; i < tree.Depth() - 1; i++) {
    for (int j = 0; j < (1 << i); j++) {
      tree.SetType(p, T::NodeTypeContainer);
      p++;
    }
  }
  for (int j = 0; j < (1 << (tree.Depth() - 1)); j++) {
    tree.SetType(p, T::NodeTypeData);
    p++;
  }
  return p;
}

template <class T>
void TestTreeSetGet(T & tree, std::string className) {
  ScopedPass scope;
  
  std::cout << "testing " << className << "::[Set/Get]() ... ";
  
  // allocate every node
  ANAlloc::Path p = AllocAll(tree);
  
  // verify that the tree is the way we set it up to be
  p = 0;
  for (int i = 0; i < tree.Depth() - 1; i++) {
    for (int j = 0; j < (1 << i); j++) {
      assert(tree.GetType(p) == T::NodeTypeContainer);
      p++;
    }
  }
  for (int j = 0; j < (1 << (tree.Depth() - 1)); j++) {
    assert(tree.GetType(p) == T::NodeTypeData);
    p++;
  }
    
  // try freeing a base node
  tree.SetType(p - 1, T::NodeTypeFree);
  assert(tree.GetType(p - 1) == T::NodeTypeFree);
  
  // attempt to free it's sibling and parent
  if (tree.Depth() > 1) {
    tree.SetType(p - 2, T::NodeTypeFree);
    ANAlloc::Path parent = ANAlloc::PathParent(p - 1);
    tree.SetType(parent, T::NodeTypeFree);
    assert(tree.GetType(p - 1) == T::NodeTypeFree);
    assert(tree.GetType(p - 2) == T::NodeTypeFree);
    assert(tree.GetType(parent) == T::NodeTypeFree);
  }
}

template <class T>
void TestTreeFindFree(T & tree, std::string className) {
  ScopedPass scope;
  
  std::cout << "testing " << className << "::FindFree() ... ";
  
  FreeAll(tree);
  
  // make sure the first chunk is the biggest chunk
  ANAlloc::Path p;
  for (int i = 0; i < tree.Depth(); i++) {
    bool result = tree.FindFree(i, p);
    assert(result);
    assert(p == 0);
  }
  
  if (tree.Depth() < 2) return;
  
  // allocate the root node, and create a child data node
  tree.SetType(0, T::NodeTypeContainer);
  tree.SetType(1, T::NodeTypeData);
  
  assert(!tree.FindFree(0, p));
  
  for (int i = 1; i < tree.Depth(); i++) {
    bool result = tree.FindFree(i, p);
    assert(result);
    assert(p == 2);
  }
  
  // for each level under this, allocate chunks of a certain size and then
  // allocate the left child of each chunk.
  for (int i = 1; i < tree.Depth(); i++) {
    ANAlloc::Path minPath = (1L << i) - 1;
    ANAlloc::Path maxPath = (1L << (i + 1)) - 1;
    bool result = tree.FindFree(i, p);
    assert(result);
    assert(p >= minPath && p < maxPath);
    assert(!(p & 1));
    if (i + 1 < tree.Depth()) {
      tree.SetType(p, T::NodeTypeContainer);
      tree.SetType(ANAlloc::PathLeft(p), T::NodeTypeData);
      tree.SetType(ANAlloc::PathRight(p), T::NodeTypeFree);
    } else {
      tree.SetType(p, T::NodeTypeData);
    }
    assert(!tree.FindFree(i, p));
  }
  
  // verify nothing is left
  assert(!tree.FindFree(tree.Depth() - 1, p));
}
