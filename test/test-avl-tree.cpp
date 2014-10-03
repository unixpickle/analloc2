#include "scoped-pass.hpp"
#include "posix-virtual-aligner.hpp"
#include <analloc2>

using namespace analloc;

void TestBalancedInsertions();

int main() {
  TestBalancedInsertions();
  return 0;
}

void TestBalancedInsertions() {
  ScopedPass pass("AvlTree<int>::[Add/FindEqual]() [pre-balanced]");
  
  PosixVirtualAligner aligner;
  AvlTree<int> tree(aligner);
  
  auto * node3 = tree.Add(3);
  auto * node1 = tree.Add(1);
  auto * node5 = tree.Add(5);
  auto * node0 = tree.Add(0);
  auto * node2 = tree.Add(2);
  auto * node4 = tree.Add(4);
  auto * node6 = tree.Add(6);
  
  assert(tree.GetRoot() == node3);
  assert(tree.GetRoot()->GetLeft() == node1);
  assert(tree.GetRoot()->GetRight() == node5);
  assert(tree.GetRoot()->GetLeft()->GetLeft() == node0);
  assert(tree.GetRoot()->GetLeft()->GetRight() == node2);
  assert(tree.GetRoot()->GetRight()->GetLeft() == node4);
  assert(tree.GetRoot()->GetRight()->GetRight() == node6);
  assert(tree.FindEqual(0) == node0);
  assert(tree.FindEqual(1) == node1);
  assert(tree.FindEqual(2) == node2);
  assert(tree.FindEqual(3) == node3);
  assert(tree.FindEqual(4) == node4);
  assert(tree.FindEqual(5) == node5);
  assert(tree.FindEqual(6) == node6);
  assert(!tree.FindEqual(7));
  assert(!tree.FindEqual(-1));
}
