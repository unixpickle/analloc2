#include "tree_test.hpp"

template <class T>
class AllocTest {
private:
  ANAlloc::Allocator<T> alloc;
  T tree;
  std::string typeName;
public:
  AllocTest(ANAlloc::Allocator<T> & a, T & t, std::string n)
    : alloc(a), tree(t), typeName(n) {}
  
  void TestBaseAlloc();
  void TestFragAlloc();
  void TestSplit();
  void TestFind();
  void TestReserve();
};

template <class T>
void AllocTest<T>::TestBaseAlloc() {
  ScopedPass scope;
  std::cout << "testing Allocator<" 
    << typeName << ">::[Alloc/Free]() [base] ... ";
  
  FreeAll(tree);
  
  ANAlloc::Path p;
  alloc.Alloc(tree.Depth() - 1, p);
  assert((1 << (tree.Depth() - 1)) - 1 == p);
  
  ANAlloc::Path idx = 2;
  for (int i = 1; i < tree.Depth(); i++) {
    if (i == tree.Depth() - 1) {
      assert(tree.GetType(idx - 1) == T::NodeTypeData);
    } else {
      assert(tree.GetType(idx - 1) == T::NodeTypeContainer);
    }
    assert(tree.GetType(idx) == T::NodeTypeFree);
    idx <<= 1;
  }
  
  alloc.Free(p);
  assert(tree.GetType(0) == T::NodeTypeFree);
}

template <class T>
void AllocTest<T>::TestFragAlloc() {
  ScopedPass scope;
  std::cout << "testing Allocator<" << typeName
    << ">::[Alloc/Free]() ... ";
  
  FreeAll(tree);
  
  // make a tree that looks like this:
  /******************************
   *             +              *
   *     -               +      *
   *                 +       -  *
   *                - -         *
   ******************************/
  
  // attempt to allocate the left guy
  ANAlloc::Path p;
  bool res = alloc.Alloc(1, p);
  assert(res);
  assert(p == 1);
  assert(tree.GetType(0) == T::NodeTypeContainer);
  assert(tree.GetType(1) == T::NodeTypeData);
  assert(tree.GetType(2) == T::NodeTypeFree);
  
  // allocate the two right children
  res = alloc.Alloc(2, p);
  assert(res);
  assert(p == 5);
  assert(tree.GetType(2) == T::NodeTypeContainer);
  assert(tree.GetType(5) == T::NodeTypeData);
  assert(tree.GetType(6) == T::NodeTypeFree);
  
  res = alloc.Alloc(2, p);
  assert(res);
  assert(p == 6);
  assert(tree.GetType(2) == T::NodeTypeContainer);
  assert(tree.GetType(5) == T::NodeTypeData);
  assert(tree.GetType(6) == T::NodeTypeData);
  
  // try freeing and reallocating right right (6)
  alloc.Free(6);
  assert(tree.GetType(2) == T::NodeTypeContainer);
  assert(tree.GetType(5) == T::NodeTypeData);
  assert(tree.GetType(6) == T::NodeTypeFree);
  
  res = alloc.Alloc(2, p);
  assert(res);
  assert(p == 6);
  assert(tree.GetType(2) == T::NodeTypeContainer);
  assert(tree.GetType(5) == T::NodeTypeData);
  assert(tree.GetType(6) == T::NodeTypeData);
  
  // allocate more empty stuff under right left (5)
  alloc.Free(5);
  res = alloc.Alloc(3, p);
  assert(res);
  assert(p == 11);
  
  res = alloc.Alloc(3, p);
  assert(res);
  assert(p == 12);
  
  // verify the tree
  assert(tree.GetType(0) == T::NodeTypeContainer);
  assert(tree.GetType(1) == T::NodeTypeData);
  assert(tree.GetType(2) == T::NodeTypeContainer);
  assert(tree.GetType(5) == T::NodeTypeContainer);
  assert(tree.GetType(6) == T::NodeTypeData);
  assert(tree.GetType(11) == T::NodeTypeData);
  assert(tree.GetType(12) == T::NodeTypeData);
  
  // make sure freeing works
  alloc.Free(12);
  alloc.Free(11);
  alloc.Free(6);
  alloc.Free(1);
  assert(tree.GetType(0) == T::NodeTypeFree);
}

template <class T>
void AllocTest<T>::TestSplit() {
  ScopedPass scope;
  std::cout << "testing Allocator<" 
    << typeName << ">::Split() ... ";
  
  FreeAll(tree);
  
  ANAlloc::Path p;
  alloc.Alloc(0, p);
  assert(p == 0);
  assert(tree.GetType(0) == T::NodeTypeData);
  
  alloc.Split(0);
  assert(tree.GetType(0) == T::NodeTypeContainer);
  assert(tree.GetType(1) == T::NodeTypeData);
  assert(tree.GetType(2) == T::NodeTypeData);
  
  alloc.Free(2);
  
  // continually split the path and free the right node
  p = 1;
  for (int i = 1; i < tree.Depth() - 1; i++) {
    alloc.Split(p);
    ANAlloc::Path left = ANAlloc::PathLeft(p);
    ANAlloc::Path right = ANAlloc::PathRight(p);
    assert(tree.GetType(p) == T::NodeTypeContainer);
    assert(tree.GetType(left) == T::NodeTypeData);
    assert(tree.GetType(right) == T::NodeTypeData);
    alloc.Free(right);
    p = left;
  }
  
  alloc.Free(p);
  assert(tree.GetType(0) == T::NodeTypeFree);
}

template <class T>
void AllocTest<T>::TestFind() {
  ScopedPass scope;
  std::cout << "testing Allocator<" 
    << typeName << ">::Find() ... ";
  
  FreeAll(tree);
  
  ANAlloc::Path p;
  bool res = alloc.Alloc(1, p);
  assert(res);
  assert(p == 1);
  
  uintptr_t secondLast = (1L << (tree.Depth() - 2));
  
  ANAlloc::Path result = 0;
  res = alloc.Find(0, result);
  assert(res);
  assert(result == 1);
  
  res = alloc.Find(secondLast, result);
  assert(!res);
  
  res = alloc.Find(secondLast - 1, result);
  assert(res);
  assert(result == 1);
  
  alloc.Free(1);
  
  // allocate a base level element
  res = alloc.Alloc(tree.Depth() - 1, p);
  assert(res);
  assert(p == (1 << (tree.Depth() - 1)) - 1);
  res = alloc.Find(0, result);
  assert(res);
  assert(result == p);
  res = alloc.Find(1, result);
  assert(!res);
  res = alloc.Find(2, result);
  assert(!res);
}

template <class T>
void AllocTest<T>::TestReserve() {
  ScopedPass scope;
  std::cout << "testing Allocator<" 
    << typeName << ">::Reserve() ... ";
  
  FreeAll(tree);
  
  ANAlloc::Path p;
  bool res = alloc.Alloc(0, p);
  assert(res);
  assert(p == 0);
  
  uintptr_t baseCount = 1 << (tree.Depth() - 1);
  alloc.Reserve(p, (baseCount >> 1) - 1, 2);
  
  // make sure that the two middle base nodes were grabbed properly
  ANAlloc::Path leftPath = 1;
  ANAlloc::Path rightPath = 2;
  assert(tree.Depth() >= 2);
  assert(tree.GetType(0) == T::NodeTypeContainer);
  assert(tree.GetType(1) == T::NodeTypeContainer);
  assert(tree.GetType(2) == T::NodeTypeContainer);
  
  for (int i = 1; i < tree.Depth() - 1; i++) {
    typename T::NodeType type;
    if (i + 2 == tree.Depth()) {
      type = T::NodeTypeData;
    } else {
      type = T::NodeTypeContainer;
    }
        
    ANAlloc::Path left = ANAlloc::PathLeft(rightPath);
    assert(tree.GetType(left) == type);
    assert(tree.GetType(left + 1) == T::NodeTypeFree);
    
    ANAlloc::Path right = ANAlloc::PathRight(leftPath);
    assert(tree.GetType(right) == type);
    assert(tree.GetType(right - 1) == T::NodeTypeFree);
    
    leftPath = right;
    rightPath = left;
  }
  
  FreeAll(tree);
  
  // try and grab every base node but the far left one
  res = alloc.Alloc(0, p);
  assert(res);
  assert(p == 0);
  
  alloc.Reserve(p, 1, baseCount - 1);

  for (int i = 0; i < tree.Depth() - 1; i++) {
    ANAlloc::Path right = ANAlloc::PathRight(p);
    assert(tree.GetType(p) == T::NodeTypeContainer);
    assert(tree.GetType(right) == T::NodeTypeData);
    if (i == tree.Depth() - 2) {
      assert(tree.GetType(right - 1) == T::NodeTypeFree);
    } else {
      assert(tree.GetType(right - 1) == T::NodeTypeContainer);
    }
    p = right - 1;
  }
}
