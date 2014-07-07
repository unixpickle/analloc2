#include "tree-tester.hpp"
#include "scoped-pass.hpp"

namespace ANAlloc {

TreeTester::TreeTester(Tree & _tree, const char * _name)
  : name(_name), tree(_tree) {
}

void TreeTester::AllocAll() {
  tree.FreeAll();
  for (int i = 0; i < tree.GetDepth() - 1; i++) {
    for (uint64_t j = 0; j < Path::DepthCount(i); j++) {
      Path p(i, j);
      tree.SetType(p, NodeTypeContainer);
    }
  }
  for (uint64_t j = 0; j < Path::DepthCount(tree.GetDepth() - 1); j++) {
    Path p(tree.GetDepth() - 1, j);
    tree.SetType(p, NodeTypeData);
  }
}

void TreeTester::TestAllocAll() {
  ScopedPass pass(name, "::[Set/Get]() [AllocAll]");

  // allocate every node
  AllocAll();

  // verify that the tree is the way we set it up to be
  for (int i = 0; i < tree.GetDepth() - 1; i++) {
    for (uint64_t j = 0; j < Path::DepthCount(i); j++) {
      Path p(i, j);
      assert(tree.GetType(p) == NodeTypeContainer);
    }
  }
  for (uint64_t j = 0; j < Path::DepthCount(tree.GetDepth() - 1); j++) {
    Path p(tree.GetDepth() - 1, j);
    assert(tree.GetType(p) == NodeTypeData);
  }
}

void TreeTester::TestSetGet() {
  ScopedPass pass(name, "::[Set/Get]()");

  // allocate every node
  AllocAll();
  
  // try freeing a base node
  Path lastNode(tree.GetDepth() - 1,
                Path::DepthCount(tree.GetDepth() - 1) - 1);
  tree.SetType(lastNode, NodeTypeFree);
  assert(tree.GetType(lastNode) == NodeTypeFree);

  // attempt to free it's sibling and parent
  if (tree.GetDepth() > 1) {
    Path secondToLast = lastNode.Sibling();
    
    tree.SetType(secondToLast, NodeTypeFree);
    Path parent = lastNode.Parent();
    
    tree.SetType(parent, NodeTypeFree);
    
    assert(tree.GetType(parent) == NodeTypeFree);
  }
}

void TreeTester::TestFindFree() {
  ScopedPass pass(name, "::FindFree()");
  
  tree.FreeAll();

  // make sure the first chunk is the biggest chunk
  Path p;
  for (int i = 0; i < tree.GetDepth(); i++) {
    bool result = tree.FindFree(i, p);
    assert(result);
    assert(p == Path::Root());
  }

  if (tree.GetDepth() < 2) return;

  // allocate the root node, and create a child data node
  tree.SetType(Path(0, 0), NodeTypeContainer);
  tree.SetType(Path(1, 0), NodeTypeData);
  tree.SetType(Path(1, 1), NodeTypeFree);

  assert(!tree.FindFree(0, p));

  for (int i = 1; i < tree.GetDepth(); i++) {
    bool result = tree.FindFree(i, p);
    assert(result);
    assert(p == Path(1, 1));
  }

  // for each level under this, allocate chunks of a certain size and then
  // allocate the left child of each chunk.
  for (int i = 1; i < tree.GetDepth(); i++) {
    bool result = tree.FindFree(i, p);
    assert(result);
    assert(p.GetDepth() == i);
    assert(p.GetIndex() % 2 != 0);
    if (i + 1 < tree.GetDepth()) {
      tree.SetType(p, NodeTypeContainer);
      tree.SetType(p.Left(), NodeTypeData);
      tree.SetType(p.Right(), NodeTypeFree);
    } else {
      tree.SetType(p, NodeTypeData);
    }
    assert(!tree.FindFree(i, p));
  }

  // verify nothing is left
  assert(!tree.FindFree(tree.GetDepth() - 1, p));
}

void TreeTester::TestBaseAlloc() {
  ScopedPass pass(name, "::Alloc() [base node]");
  tree.FreeAll();
  
  Path p;
  bool res = tree.Alloc(tree.GetDepth() - 1, p);
  assert(res);
  assert(p.GetDepth() == tree.GetDepth() - 1);
  assert(tree.GetType(p) == NodeTypeData);
  assert(tree.GetType(p.Sibling()) == NodeTypeFree);
  
  p = p.Parent();
  while (p.GetDepth()) {
    assert(tree.GetType(p) == NodeTypeContainer);
    assert(tree.GetType(p.Sibling()) == NodeTypeFree);
    p = p.Parent();
  }
  assert(tree.GetType(p) == NodeTypeContainer);
}

void TreeTester::TestFragAlloc() {
  ScopedPass pass(name, "::Alloc()");
  tree.FreeAll();
  
  Path d1Path, d2Path, d3Path1, d3Path2;
  bool res = tree.Alloc(1, d1Path);
  assert(res);
  assert(tree.GetType(Path::Root()) == NodeTypeContainer);
  assert(tree.GetType(d1Path) == NodeTypeData);
  assert(tree.GetType(d1Path.Sibling()) == NodeTypeFree);
  
  res = tree.Alloc(2, d2Path);
  assert(res);
  assert(d2Path.Parent() == d1Path.Sibling());
  assert(tree.GetType(d1Path.Sibling()) == NodeTypeContainer);
  assert(tree.GetType(d2Path) == NodeTypeData);
  assert(tree.GetType(d2Path.Sibling()) == NodeTypeFree);
  
  res = tree.Alloc(3, d3Path1);
  assert(res);
  assert(d3Path1.Parent() == d2Path.Sibling());
  assert(tree.GetType(d2Path.Sibling()) == NodeTypeContainer);
  assert(tree.GetType(d3Path1) == NodeTypeData);
  assert(tree.GetType(d3Path1.Sibling()) == NodeTypeFree);
  
  res = tree.Alloc(3, d3Path2);
  assert(res);
  assert(d3Path2.Sibling() == d3Path1);
  
  // verify the tree structure as a whole
  assert(tree.GetType(Path::Root()) == NodeTypeContainer);
  assert(tree.GetType(d1Path) == NodeTypeData);
  assert(tree.GetType(d1Path.Sibling()) == NodeTypeContainer);
  assert(tree.GetType(d2Path) == NodeTypeData);
  assert(tree.GetType(d2Path.Sibling()) == NodeTypeContainer);
  assert(tree.GetType(d3Path1) == NodeTypeData);
  assert(tree.GetType(d3Path2) == NodeTypeData);
  
  // attempt reverse-order allocation of d3Path
  tree.Dealloc(d3Path1);
  assert(tree.GetType(d3Path2) == NodeTypeData);
  assert(tree.GetType(d3Path1) == NodeTypeFree);
  res = tree.Alloc(3, d3Path1);
  assert(res);
  assert(d3Path1 == d3Path2.Sibling());
  
  // re-verify the tree structure as a whole
  assert(tree.GetType(Path::Root()) == NodeTypeContainer);
  assert(tree.GetType(d1Path) == NodeTypeData);
  assert(tree.GetType(d1Path.Sibling()) == NodeTypeContainer);
  assert(tree.GetType(d2Path) == NodeTypeData);
  assert(tree.GetType(d2Path.Sibling()) == NodeTypeContainer);
  assert(tree.GetType(d3Path1) == NodeTypeData);
  assert(tree.GetType(d3Path2) == NodeTypeData);
}

void TreeTester::TestFindByShadow() {
  ScopedPass pass(name, "::FindByShadow()");
  tree.FreeAll();
  
  bool res;
  Path p;
  uint64_t baseCount = Path::DepthCount(tree.GetDepth() - 1);
  
  tree.SetType(Path::Root(), NodeTypeData);
  for (uint64_t i = 0; i < baseCount; i++) {
    res = tree.FindByShadow(i, p);
    assert(res);
    assert(p == Path::Root());
  }
  
  tree.SetType(Path::Root(), NodeTypeContainer);
  tree.SetType(Path(1, 0), NodeTypeData);
  tree.SetType(Path(1, 1), NodeTypeFree);
  for (uint64_t i = 0; i < baseCount; i++) {
    res = tree.FindByShadow(i, p);
    assert(res == (i < baseCount / 2));
    if (i < baseCount / 2) {
      assert(p == Path(1, 0));
    }
  }
  
  tree.SetType(Path(1, 0), NodeTypeContainer);
  tree.SetType(Path(2, 0), NodeTypeFree);
  tree.SetType(Path(2, 1), NodeTypeData);
  
  for (uint64_t i = 0; i < baseCount; i++) {
    res = tree.FindByShadow(i, p);
    if (i < baseCount / 4 || i >= baseCount / 2) {
      assert(!res);
    } else {
      assert(res);
      assert(p == Path(2, 1));
    }
  }
}

void TreeTester::TestCarveCenter() {
  ScopedPass pass(name, "::Carve() [center]");
  tree.FreeAll();
  
  uint64_t shadowCount = Path::DepthCount(tree.GetDepth() - 1);
  
  // carve in the two middle nodes
  tree.SetType(Path::Root(), NodeTypeData);
  tree.Carve(Path::Root(), shadowCount / 2 - 1, 2);
  
  Path p(1, 0);
  while (p.GetDepth() < tree.GetDepth() - 1) {
    assert(tree.GetType(p) == NodeTypeContainer);
    assert(tree.GetType(p.Left()) == NodeTypeFree);
    p = p.Right();
  }
  assert(tree.GetType(p) == NodeTypeData);
  
  p = Path(1, 1);
  while (p.GetDepth() < tree.GetDepth() - 1) {
    assert(tree.GetType(p) == NodeTypeContainer);
    assert(tree.GetType(p.Right()) == NodeTypeFree);
    p = p.Left();
  }
  assert(tree.GetType(p) == NodeTypeData);
}

void TreeTester::TestCarveSide() {
  ScopedPass pass(name, "::Carve() [side]");
  tree.FreeAll();
  
  uint64_t shadowCount = Path::DepthCount(tree.GetDepth() - 1);
  
  // carve in the two middle nodes
  tree.SetType(Path::Root(), NodeTypeContainer);
  tree.SetType(Path(1, 0), NodeTypeData);
  tree.SetType(Path(1, 1), NodeTypeFree);
  
  // carve all but rightmost left node
  tree.Carve(Path(1, 0), 0, shadowCount / 2 - 1);
  
  Path p(1, 0);
  while (p.GetDepth() < tree.GetDepth() - 1) {
    assert(tree.GetType(p) == NodeTypeContainer);
    assert(tree.GetType(p.Left()) == NodeTypeData);
    p = p.Right();
  }
  assert(tree.GetType(p) == NodeTypeFree);
}

void TreeTester::TestCarveFull() {
  ScopedPass pass(name, "::Carve() [full]");
  tree.FreeAll();
  
  uint64_t shadowCount = Path::DepthCount(tree.GetDepth() - 1);
  
  tree.SetType(Path::Root(), NodeTypeData);
  tree.Carve(Path::Root(), 0, shadowCount);
  assert(tree.GetType(Path::Root()) == NodeTypeData);
}

void TreeTester::TestAll() {
  TestAllocAll();
  TestSetGet();
  TestFindFree();
  TestBaseAlloc();
  TestFragAlloc();
  TestFindByShadow();
  TestCarveCenter();
  TestCarveSide();
  TestCarveFull();
}

}
