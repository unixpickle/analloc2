#include "tree-tester.hpp"
#include "scoped-pass.hpp"

namespace ANAlloc {

TreeTester::TreeTester(Tree & _tree, const char * _name)
  : name(_name), tree(_tree) {
}
  
void TreeTester::FreeAll() {
  FreeAll(Path::RootPath());
}

void TreeTester::FreeAll(Path p) {
  NodeType type = tree.GetType(p);
  if (type != NodeTypeFree) {
    if (type == NodeTypeContainer) {
      FreeAll(p.Left());
      FreeAll(p.Right());
    }
    tree.SetType(p, NodeTypeFree);
  }
}

void TreeTester::AllocAll() {
  FreeAll();
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
  
  FreeAll();

  // make sure the first chunk is the biggest chunk
  Path p;
  for (int i = 0; i < tree.GetDepth(); i++) {
    bool result = tree.FindFree(i, p);
    assert(result);
    assert(p == Path::RootPath());
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

}
