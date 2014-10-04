#include "scoped-pass.hpp"
#include "posix-virtual-aligner.hpp"
#include <analloc2>

using namespace analloc;

PosixVirtualAligner aligner;

void TestBalancedInsertions();
void TestBalancedDeletions();
void TestBasicRootUnbalancedInsertions();
void TestBasicNonrootUnbalancedInsertions();
void TestRandomInsertions();
void TestBalancedTrivialDeletions();

bool IsLeaf(const AvlNode<int> * node);
bool IsFull(const AvlNode<int> * node);
bool IsLeftOnly(const AvlNode<int> * node);
bool IsRightOnly(const AvlNode<int> * node);
bool ValidateParent(const AvlNode<int> * node);

int main() {
  TestBalancedInsertions();
  assert(aligner.GetAllocCount() == 0);
  TestBalancedDeletions();
  assert(aligner.GetAllocCount() == 0);
  TestBasicRootUnbalancedInsertions();
  assert(aligner.GetAllocCount() == 0);
  TestBasicNonrootUnbalancedInsertions();
  assert(aligner.GetAllocCount() == 0);
  TestRandomInsertions();
  assert(aligner.GetAllocCount() == 0);
  TestBalancedTrivialDeletions();
  assert(aligner.GetAllocCount() == 0);
  return 0;
}

void TestBalancedInsertions() {
  ScopedPass pass("AvlTree<int>::Add() [balanced]");
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
  ScopedPass pass("AvlTree<int>::Remove() [leafs only]");
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
    assert(IsFull(tree.GetRoot()));
    assert(IsLeaf(tree.GetRoot()->left));
    assert(IsLeaf(tree.GetRoot()->right));
    assert(tree.GetRoot()->parent == nullptr);
    assert(ValidateParent(tree.GetRoot()->left));
    assert(ValidateParent(tree.GetRoot()->right));
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
      assert(tree.GetRoot() != nullptr);
      assert(tree.GetRoot()->parent == nullptr);
      assert(IsFull(tree.GetRoot()));
      assert(ValidateParent(tree.GetRoot()->right));
      if (j == 0) {
        assert(tree.GetRoot()->GetValue() == 4);
        assert(tree.GetRoot()->right->GetValue() == 5);
        rotated = tree.GetRoot()->left;
      } else {
        assert(tree.GetRoot()->GetValue() == 0);
        assert(tree.GetRoot()->left->GetValue() == -1);
        rotated = tree.GetRoot()->right;
      }
      assert(IsFull(rotated));
      assert(ValidateParent(rotated));
      assert(ValidateParent(rotated->left));
      assert(ValidateParent(rotated->right));
      assert(rotated->GetValue() == 2);
      assert(rotated->left->GetValue() == 1);
      assert(rotated->right->GetValue() == 3);
      tree.Clear();
      assert(!tree.GetRoot());
      assert(aligner.GetAllocCount() == 0);
    }
  }
}

void TestRandomInsertions() {
  ScopedPass pass("AvlTree<int>::Add() [pre-randomized]");
  AvlTree<int> tree(aligner);
  
  int values[] = {5, 6, 9, 11, 14, 15, 10, 17, 3, 7, 1, 8, 16, 4, 12, 13, 2};
  static_assert(sizeof(values) == sizeof(int) * 17, "Invalid random values");
  for (int i = 0; i < 17; ++i) {
    assert(aligner.GetAllocCount() == (size_t)i);
    tree.Add(values[i]);
    assert(aligner.GetAllocCount() == (size_t)i + 1);
    if (i == 5) {
      assert(tree.GetRoot() != nullptr);
      assert(tree.GetRoot()->parent == nullptr);
      assert(IsFull(tree.GetRoot()));
      assert(IsFull(tree.GetRoot()->left));
      assert(IsRightOnly(tree.GetRoot()->right));
      assert(IsLeaf(tree.GetRoot()->right->right));
      assert(IsLeaf(tree.GetRoot()->left->left));
      assert(IsLeaf(tree.GetRoot()->left->right));
      assert(ValidateParent(tree.GetRoot()->left));
      assert(ValidateParent(tree.GetRoot()->right));
      assert(ValidateParent(tree.GetRoot()->left->left));
      assert(ValidateParent(tree.GetRoot()->left->right));
      assert(ValidateParent(tree.GetRoot()->right->right));
      assert(tree.GetRoot()->GetValue() == 11);
      assert(tree.GetRoot()->left->GetValue() == 6);
      assert(tree.GetRoot()->right->GetValue() == 14);
      assert(tree.GetRoot()->left->left->GetValue() == 5);
      assert(tree.GetRoot()->left->right->GetValue() == 9);
      assert(tree.GetRoot()->right->right->GetValue() == 15);
    } else if (i == 9) {
      assert(tree.GetRoot() != nullptr);
      assert(tree.GetRoot()->parent == nullptr);
      assert(IsFull(tree.GetRoot()));
      assert(IsFull(tree.GetRoot()->left));
      assert(IsFull(tree.GetRoot()->right));
      assert(IsLeftOnly(tree.GetRoot()->left->left));
      assert(IsFull(tree.GetRoot()->left->right));
      assert(IsLeaf(tree.GetRoot()->right->left));
      assert(IsLeaf(tree.GetRoot()->right->right));
      assert(IsLeaf(tree.GetRoot()->left->left->left));
      assert(IsLeaf(tree.GetRoot()->left->right->left));
      assert(IsLeaf(tree.GetRoot()->left->right->right));
      assert(ValidateParent(tree.GetRoot()->left));
      assert(ValidateParent(tree.GetRoot()->right));
      assert(ValidateParent(tree.GetRoot()->left->left));
      assert(ValidateParent(tree.GetRoot()->left->right));
      assert(ValidateParent(tree.GetRoot()->right->left));
      assert(ValidateParent(tree.GetRoot()->right->right));
      assert(ValidateParent(tree.GetRoot()->left->left->left));
      assert(ValidateParent(tree.GetRoot()->left->right->left));
      assert(ValidateParent(tree.GetRoot()->left->right->right));
      assert(tree.GetRoot()->GetValue() == 11);
      assert(tree.GetRoot()->left->GetValue() == 6);
      assert(tree.GetRoot()->right->GetValue() == 15);
      assert(tree.GetRoot()->left->left->GetValue() == 5);
      assert(tree.GetRoot()->left->right->GetValue() == 9);
      assert(tree.GetRoot()->right->left->GetValue() == 14);
      assert(tree.GetRoot()->right->right->GetValue() == 17);
      assert(tree.GetRoot()->left->left->left->GetValue() == 3);
      assert(tree.GetRoot()->left->right->left->GetValue() == 7);
      assert(tree.GetRoot()->left->right->right->GetValue() == 10);
    } else if (i == 16) {
      assert(tree.GetRoot() != nullptr);
      assert(tree.GetRoot()->parent == nullptr);
      assert(IsFull(tree.GetRoot()));
      assert(IsFull(tree.GetRoot()->left));
      assert(IsFull(tree.GetRoot()->right));
      // Depth 2
      assert(IsFull(tree.GetRoot()->left->left));
      assert(IsRightOnly(tree.GetRoot()->left->right));
      assert(IsFull(tree.GetRoot()->right->left));
      assert(IsLeftOnly(tree.GetRoot()->right->right));
      // Depth 3
      assert(IsRightOnly(tree.GetRoot()->left->left->left));
      assert(IsLeftOnly(tree.GetRoot()->left->left->right));
      assert(IsLeaf(tree.GetRoot()->left->right->right));
      assert(IsLeaf(tree.GetRoot()->right->left->left));
      assert(IsFull(tree.GetRoot()->right->left->right));
      assert(IsLeaf(tree.GetRoot()->right->right->left));
      // Depth 4
      assert(IsLeaf(tree.GetRoot()->left->left->left->right));
      assert(IsLeaf(tree.GetRoot()->left->left->right->left));
      assert(IsLeaf(tree.GetRoot()->right->left->right->left));
      assert(IsLeaf(tree.GetRoot()->right->left->right->right));
      // Parental validation
      assert(ValidateParent(tree.GetRoot()->left));
      assert(ValidateParent(tree.GetRoot()->right));
      assert(ValidateParent(tree.GetRoot()->left->left));
      assert(ValidateParent(tree.GetRoot()->left->right));
      assert(ValidateParent(tree.GetRoot()->right->left));
      assert(ValidateParent(tree.GetRoot()->right->right));
      assert(ValidateParent(tree.GetRoot()->left->left->left));
      assert(ValidateParent(tree.GetRoot()->left->left->right));
      assert(ValidateParent(tree.GetRoot()->left->right->right));
      assert(ValidateParent(tree.GetRoot()->right->left->left));
      assert(ValidateParent(tree.GetRoot()->right->left->right));
      assert(ValidateParent(tree.GetRoot()->right->right->left));
      assert(ValidateParent(tree.GetRoot()->left->left->left->right));
      assert(ValidateParent(tree.GetRoot()->left->left->right->left));
      assert(ValidateParent(tree.GetRoot()->right->left->right->left));
      assert(ValidateParent(tree.GetRoot()->right->left->right->right));
      // Value validation
      assert(tree.GetRoot()->GetValue() == 9);
      assert(tree.GetRoot()->left->GetValue() == 6);
      assert(tree.GetRoot()->right->GetValue() == 15);
      assert(tree.GetRoot()->left->left->GetValue() == 3);
      assert(tree.GetRoot()->left->right->GetValue() == 7);
      assert(tree.GetRoot()->right->left->GetValue() == 11);
      assert(tree.GetRoot()->right->right->GetValue() == 17);
      assert(tree.GetRoot()->left->left->left->GetValue() == 1);
      assert(tree.GetRoot()->left->left->right->GetValue() == 5);
      assert(tree.GetRoot()->left->right->right->GetValue() == 8);
      assert(tree.GetRoot()->right->left->left->GetValue() == 10);
      assert(tree.GetRoot()->right->left->right->GetValue() == 13);
      assert(tree.GetRoot()->right->right->left->GetValue() == 16);
      assert(tree.GetRoot()->left->left->left->right->GetValue() == 2);
      assert(tree.GetRoot()->left->left->right->left->GetValue() == 4);
      assert(tree.GetRoot()->right->left->right->left->GetValue() == 12);
      assert(tree.GetRoot()->right->left->right->right->GetValue() == 14);
    }
  }
}

void TestBalancedTrivialDeletions() {
  ScopedPass pass("AvlTree<int>::Remove() [balanced, trivial]");
  AvlTree<int> tree(aligner);
  
  // Trivial deletions with no subnodes
  tree.Add(2);
  tree.Add(1);
  tree.Add(3);
  assert(aligner.GetAllocCount() == 3);
  tree.Remove(3);
  assert(aligner.GetAllocCount() == 2);
  assert(tree.GetRoot() != nullptr);
  assert(tree.GetRoot()->parent == nullptr);
  assert(IsLeftOnly(tree.GetRoot()));
  assert(IsLeaf(tree.GetRoot()->left));
  assert(ValidateParent(tree.GetRoot()->left));
  assert(tree.GetRoot()->GetValue() == 2);
  assert(tree.GetRoot()->left->GetValue() == 1);
  tree.Remove(1);
  assert(aligner.GetAllocCount() == 1);
  assert(tree.GetRoot() != nullptr);
  assert(tree.GetRoot()->parent == nullptr);
  assert(IsLeaf(tree.GetRoot()));
  assert(tree.GetRoot()->GetValue() == 2);
  tree.Remove(2);
  assert(aligner.GetAllocCount() == 0);
  assert(tree.GetRoot() == nullptr);
  
  // Rooted trivial case with right child.
  tree.Add(2);
  tree.Add(3);
  tree.Remove(2);
  assert(tree.GetRoot() != nullptr);
  assert(tree.GetRoot()->parent == nullptr);
  assert(IsLeaf(tree.GetRoot()));
  assert(tree.GetRoot()->GetValue() == 3);
  tree.Remove(3);
  assert(aligner.GetAllocCount() == 0);
  assert(tree.GetRoot() == nullptr);
  
  // Rooted trivial case with left child.
  tree.Add(2);
  tree.Add(1);
  tree.Remove(2);
  assert(tree.GetRoot() != nullptr);
  assert(tree.GetRoot()->parent == nullptr);
  assert(IsLeaf(tree.GetRoot()));
  assert(tree.GetRoot()->GetValue() == 1);
  tree.Remove(1);
  assert(aligner.GetAllocCount() == 0);
  assert(tree.GetRoot() == nullptr);
  
  // Unrooted trivial case with right child.
  tree.Add(2);
  tree.Add(1);
  tree.Add(3);
  tree.Add(4);
  assert(aligner.GetAllocCount() == 4);
  tree.Remove(3);
  assert(aligner.GetAllocCount() == 3);
  assert(tree.GetRoot() != nullptr);
  assert(tree.GetRoot()->parent == nullptr);
  assert(IsFull(tree.GetRoot()));
  assert(IsLeaf(tree.GetRoot()->left));
  assert(IsLeaf(tree.GetRoot()->right));
  assert(ValidateParent(tree.GetRoot()->left));
  assert(ValidateParent(tree.GetRoot()->right));
  assert(tree.GetRoot()->GetValue() == 2);
  assert(tree.GetRoot()->left->GetValue() == 1);
  assert(tree.GetRoot()->right->GetValue() == 4);
  tree.Clear();
  assert(aligner.GetAllocCount() == 0);
  
  // Unrooted trivial case with left child.
  tree.Add(2);
  tree.Add(1);
  tree.Add(4);
  tree.Add(3);
  assert(aligner.GetAllocCount() == 4);
  tree.Remove(4);
  assert(aligner.GetAllocCount() == 3);
  assert(tree.GetRoot() != nullptr);
  assert(tree.GetRoot()->parent == nullptr);
  assert(IsFull(tree.GetRoot()));
  assert(IsLeaf(tree.GetRoot()->left));
  assert(IsLeaf(tree.GetRoot()->right));
  assert(ValidateParent(tree.GetRoot()->left));
  assert(ValidateParent(tree.GetRoot()->right));
  assert(tree.GetRoot()->GetValue() == 2);
  assert(tree.GetRoot()->left->GetValue() == 1);
  assert(tree.GetRoot()->right->GetValue() == 3);
  tree.Clear();
  assert(aligner.GetAllocCount() == 0);
}

bool IsLeaf(const AvlNode<int> * node) {
  if (!node) return false;
  return node->left == nullptr && node->right == nullptr;
}

bool IsFull(const AvlNode<int> * node) {
  if (!node) return false;
  return node->left != nullptr && node->right != nullptr;
}

bool IsLeftOnly(const AvlNode<int> * node) {
  if (!node) return false;
  return node->left != nullptr && node->right == nullptr;
}

bool IsRightOnly(const AvlNode<int> * node) {
  if (!node) return false;
  return node->left == nullptr && node->right != nullptr;
}

bool ValidateParent(const AvlNode<int> * node) {
  if (!node->parent) return true;
  return node->parent->left == node || node->parent->right == node;
}
