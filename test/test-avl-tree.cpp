#include "scoped-pass.hpp"
#include "posix-virtual-aligner.hpp"
#include <analloc2>

using namespace analloc;

void TestBalancedInsertions();
void TestBalancedDeletions();

int main() {
  TestBalancedInsertions();
  return 0;
}

void TestBalancedInsertions() {
  ScopedPass pass("AvlTree<int>::[Add/FindEqual]() [pre-balanced]");
  
  PosixVirtualAligner aligner;
  AvlTree<int> tree(aligner);
  
  assert(tree.Add(3));
  assert(tree.Add(1));
  assert(tree.Add(5));
  assert(tree.Add(0));
  assert(tree.Add(2));
  assert(tree.Add(4));
  assert(tree.Add(6));
  
  assert(tree.GetRoot()->GetValue() == 3);
  assert(tree.GetRoot()->left->GetValue() == 1);
  assert(tree.GetRoot()->right->GetValue() == 5);
  assert(tree.GetRoot()->left->left->GetValue() == 0);
  assert(tree.GetRoot()->left->right->GetValue() == 2);
  assert(tree.GetRoot()->right->left->GetValue() == 4);
  assert(tree.GetRoot()->right->right->GetValue() == 6);
  assert(tree.Contains(0));
  assert(tree.Contains(1));
  assert(tree.Contains(2));
  assert(tree.Contains(3));
  assert(tree.Contains(4));
  assert(tree.Contains(5));
  assert(tree.Contains(6));
  assert(!tree.Contains(7));
  assert(!tree.Contains(-1));
}
