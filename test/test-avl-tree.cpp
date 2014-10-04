#include "scoped-pass.hpp"
#include "posix-virtual-aligner.hpp"
#include <analloc2>

using namespace analloc;

PosixVirtualAligner aligner;

void TestBalancedInsertions();
void TestBalancedDeletions();
void TestUnbalancedInsertions();

int main() {
  TestBalancedInsertions();
  assert(aligner.GetAllocCount() == 0);
  TestBalancedDeletions();
  assert(aligner.GetAllocCount() == 0);
  TestUnbalancedInsertions();
  assert(aligner.GetAllocCount() == 0);
  return 0;
}

void TestBalancedInsertions() {
  ScopedPass pass("AvlTree<int>::Add() [pre-balanced]");
  AvlTree<int> tree(aligner);
  
  assert(tree.Add(3));
  assert(aligner.GetAllocCount() == 1);
  assert(tree.Add(1));
  assert(aligner.GetAllocCount() == 2);
  assert(tree.Add(5));
  assert(aligner.GetAllocCount() == 3);
  assert(tree.Add(0));
  assert(aligner.GetAllocCount() == 4);
  assert(tree.Add(2));
  assert(aligner.GetAllocCount() == 5);
  assert(tree.Add(4));
  assert(aligner.GetAllocCount() == 6);
  assert(tree.Add(6));
  assert(aligner.GetAllocCount() == 7);
  
  assert(tree.GetRoot()->GetValue() == 3);
  assert(tree.GetRoot()->left->GetValue() == 1);
  assert(tree.GetRoot()->right->GetValue() == 5);
  assert(tree.GetRoot()->left->left->GetValue() == 0);
  assert(tree.GetRoot()->left->right->GetValue() == 2);
  assert(tree.GetRoot()->right->left->GetValue() == 4);
  assert(tree.GetRoot()->right->right->GetValue() == 6);
  
  assert(aligner.GetAllocCount() == 7);
}

void TestBalancedDeletions() {
  ScopedPass pass("AvlTree<int>::Remove() [pre-balanced]");
  AvlTree<int> tree(aligner);
  
  assert(tree.Add(3));
  assert(tree.Add(1));
  assert(tree.Add(5));
  assert(tree.Add(0));
  assert(tree.Add(2));
  assert(tree.Add(4));
  assert(tree.Add(6));
  assert(aligner.GetAllocCount() == 7);
  
  assert(tree.GetRoot()->GetValue() == 3);
  assert(tree.GetRoot()->left->GetValue() == 1);
  assert(tree.GetRoot()->right->GetValue() == 5);
  assert(tree.GetRoot()->left->left->GetValue() == 0);
  assert(tree.GetRoot()->left->right->GetValue() == 2);
  assert(tree.GetRoot()->right->left->GetValue() == 4);
  assert(tree.GetRoot()->right->right->GetValue() == 6);
  
  // Ensure that we can't remove absent values
  assert(!tree.Remove(-1));
  assert(!tree.Remove(7));
  // Ensure that we can remove a present value
  assert(tree.Remove(6));
  assert(aligner.GetAllocCount() == 6);
  assert(tree.GetRoot()->GetValue() == 3);
  assert(tree.GetRoot()->left->GetValue() == 1);
  assert(tree.GetRoot()->right->GetValue() == 5);
  assert(tree.GetRoot()->left->left->GetValue() == 0);
  assert(tree.GetRoot()->left->right->GetValue() == 2);
  assert(tree.GetRoot()->right->left->GetValue() == 4);
  assert(tree.GetRoot()->right->right == nullptr);
  // Ensure that 6 is really gone
  assert(!tree.Remove(6));
  
  // Remove some other values one by one without affecting the balance to
  // aggressively.
  
  // Remove 4
  assert(tree.Remove(4));
  assert(aligner.GetAllocCount() == 5);
  assert(tree.GetRoot()->GetValue() == 3);
  assert(tree.GetRoot()->left->GetValue() == 1);
  assert(tree.GetRoot()->right->GetValue() == 5);
  assert(tree.GetRoot()->left->left->GetValue() == 0);
  assert(tree.GetRoot()->left->right->GetValue() == 2);
  assert(tree.GetRoot()->right->left == nullptr);
  assert(tree.GetRoot()->right->right == nullptr);
  assert(!tree.Remove(4));
  
  // Remove 0
  assert(tree.Remove(0));
  assert(aligner.GetAllocCount() == 4);
  assert(tree.GetRoot()->GetValue() == 3);
  assert(tree.GetRoot()->left->GetValue() == 1);
  assert(tree.GetRoot()->right->GetValue() == 5);
  assert(tree.GetRoot()->left->left == nullptr);
  assert(tree.GetRoot()->left->right->GetValue() == 2);
  assert(tree.GetRoot()->right->left == nullptr);
  assert(tree.GetRoot()->right->right == nullptr);
  assert(!tree.Remove(0));
  
  assert(tree.Remove(2));
  assert(aligner.GetAllocCount() == 3);
  assert(tree.GetRoot()->GetValue() == 3);
  assert(tree.GetRoot()->left->GetValue() == 1);
  assert(tree.GetRoot()->right->GetValue() == 5);
  assert(tree.GetRoot()->left->left == nullptr);
  assert(tree.GetRoot()->left->right == nullptr);
  assert(tree.GetRoot()->right->left == nullptr);
  assert(tree.GetRoot()->right->right == nullptr);
  assert(!tree.Remove(2));
  
  assert(tree.Remove(5));
  assert(aligner.GetAllocCount() == 2);
  assert(tree.GetRoot()->GetValue() == 3);
  assert(tree.GetRoot()->left->GetValue() == 1);
  assert(tree.GetRoot()->right == nullptr);
  assert(tree.GetRoot()->left->left == nullptr);
  assert(tree.GetRoot()->left->right == nullptr);
  assert(!tree.Remove(5));
  
  assert(tree.Remove(1));
  assert(aligner.GetAllocCount() == 1);
  assert(tree.GetRoot()->GetValue() == 3);
  assert(tree.GetRoot()->left == nullptr);
  assert(tree.GetRoot()->right == nullptr);
  assert(!tree.Remove(1));
  
  assert(tree.Remove(3));
  assert(aligner.GetAllocCount() == 0);
  assert(tree.GetRoot() == nullptr);
  assert(!tree.Remove(3));
}

void TestUnbalancedInsertions() {
  ScopedPass pass("AvlTree<int>::Add() [unbalanced]");
  AvlTree<int> tree(aligner);
  
  // Right-left insertion that doesn't affect root
  tree.Add(4);
  tree.Add(5);
  tree.Add(1);
  tree.Add(3);
  tree.Add(2);
  assert(aligner.GetAllocCount() == 5);
  assert(tree.GetRoot()->GetValue() == 4);
  assert(tree.GetRoot()->right->GetValue() == 5);
  assert(tree.GetRoot()->left->GetValue() == 2);
  assert(tree.GetRoot()->left->left->GetValue() == 1);
  assert(tree.GetRoot()->left->right->GetValue() == 3);
  tree.Clear();
  assert(!tree.GetRoot());
  assert(aligner.GetAllocCount() == 0);
}
