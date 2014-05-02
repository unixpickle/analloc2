#include <string>
#include <iostream>

template <class T>
void TestTreeSetGet(T & tree, std::string className) {
  std::cout << "testing " << className << "::[Set/Get]() ... ";
  
  // allocate every node
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
  
  std::cout << "passed!" << std::endl;
}

template <class T>
void TestTreeFindFree(T & tree, std::string className) {
  // TODO: here, make sure the whole tree is free
}
