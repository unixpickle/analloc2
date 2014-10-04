#include "scoped-pass.hpp"
#include "posix-virtual-aligner.hpp"
#include <analloc2>

using namespace analloc;

PosixVirtualAligner aligner;

void TestBalancedInsertions();
void TestBalancedDeletions();
void TestBasicRootUnbalancedInsertions();
void TestBasicNonrootUnbalancedInsertions();

int main() {
  TestBalancedInsertions();
  assert(aligner.GetAllocCount() == 0);
  TestBalancedDeletions();
  assert(aligner.GetAllocCount() == 0);
  TestBasicRootUnbalancedInsertions();
  assert(aligner.GetAllocCount() == 0);
  TestBasicNonrootUnbalancedInsertions();
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

void TestBasicRootUnbalancedInsertions() {
  ScopedPass pass("AvlTree<int>::Add() [unbalanced, basic, root]");
  AvlTree<int> tree(aligner);
  
  // Simple three-insertion cases
  for (int i = 0; i < 4; ++i) {
    if (i == 0) {
      // Right-right
      tree.Add(1);
      tree.Add(2);
      tree.Add(3);
    } else if (i == 1) {
      // Right-left
      tree.Add(1);
      tree.Add(3);
      tree.Add(2);
    } else if (i == 2) {
      // Left-left
      tree.Add(3);
      tree.Add(2);
      tree.Add(1);
    } else {
      // Left-right
      tree.Add(3);
      tree.Add(1);
      tree.Add(2);
    }
    assert(aligner.GetAllocCount() == 3);
    assert(tree.GetRoot() != nullptr);
    assert(tree.GetRoot()->GetValue() == 2);
    assert(tree.GetRoot()->left != nullptr);
    assert(tree.GetRoot()->right != nullptr);
    assert(tree.GetRoot()->left->GetValue() == 1);
    assert(tree.GetRoot()->right->GetValue() == 3);
    tree.Clear();
    assert(!tree.GetRoot());
    assert(aligner.GetAllocCount() == 0);
  }
}

void TestBasicNonrootUnbalancedInsertions() {
  ScopedPass pass("AvlTree<int>::Add() [unbalanced, basic, non-root]");
  AvlTree<int> tree(aligner);
  
  // Right-left insertion that doesn't affect root
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 2; ++j) {
      if (j == 0) {
        tree.Add(4);
        tree.Add(5);
      } else {
        tree.Add(0);
        tree.Add(-1);
      }
      if (i == 0) {
        // Right-right
        tree.Add(1);
        tree.Add(2);
        tree.Add(3);
      } else if (i == 1) {
        // Right-left
        tree.Add(1);
        tree.Add(3);
        tree.Add(2);
      } else if (i == 2) {
        // Left-left
        tree.Add(3);
        tree.Add(2);
        tree.Add(1);
      } else {
        // Left-right
        tree.Add(3);
        tree.Add(1);
        tree.Add(2);
      }
      assert(aligner.GetAllocCount() == 5);
      AvlNode<int> * rotated;
      if (j == 0) {
        assert(tree.GetRoot() != nullptr);
        assert(tree.GetRoot()->right != nullptr);
        assert(tree.GetRoot()->left != nullptr);
        assert(tree.GetRoot()->GetValue() == 4);
        assert(tree.GetRoot()->right->GetValue() == 5);
        rotated = tree.GetRoot()->left;
      } else {
        assert(tree.GetRoot() != nullptr);
        assert(tree.GetRoot()->right != nullptr);
        assert(tree.GetRoot()->left != nullptr);
        assert(tree.GetRoot()->GetValue() == 0);
        assert(tree.GetRoot()->left->GetValue() == -1);
        rotated = tree.GetRoot()->right;
      }
      assert(rotated->GetValue() == 2);
      assert(rotated->left != nullptr);
      assert(rotated->right != nullptr);
      assert(rotated->left->GetValue() == 1);
      assert(rotated->right->GetValue() == 3);
      tree.Clear();
      assert(!tree.GetRoot());
      assert(aligner.GetAllocCount() == 0);
    }
  }
}