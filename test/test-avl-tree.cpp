#include "scoped-pass.hpp"
#include "posix-virtual-aligner.hpp"
#include <analloc2/free-tree>

using namespace analloc;

PosixVirtualAligner aligner;

void TestBalancedInsertions();
void TestBalancedDeletions();
void TestBasicRootUnbalancedInsertions();
void TestBasicNonrootUnbalancedInsertions();
void TestRandomInsertions();
void TestBalancedTrivialDeletions();
void TestUnbalancedTrivialDeletions();
void TestBalancedNontrivialDeletions();
void TestRandomModifications();
void TestFindMethods();
void TestSearchFunction();
void TestEnumerator();

bool IsLeaf(const AvlNode<int> * node);
bool IsFull(const AvlNode<int> * node);
bool IsLeftOnly(const AvlNode<int> * node);
bool IsRightOnly(const AvlNode<int> * node);
bool ValidateParent(const AvlNode<int> * node);
bool ValidateRoot(const AvlNode<int> * node);
bool ValidateBalance(const AvlNode<int> * node, int & depthOut);

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
  TestUnbalancedTrivialDeletions();
  assert(aligner.GetAllocCount() == 0);
  TestBalancedNontrivialDeletions();
  assert(aligner.GetAllocCount() == 0);
  TestRandomModifications();
  assert(aligner.GetAllocCount() == 0);
  TestFindMethods();
  assert(aligner.GetAllocCount() == 0);
  TestSearchFunction();
  assert(aligner.GetAllocCount() == 0);
  TestEnumerator();
  assert(aligner.GetAllocCount() == 0);
  
  // TODO: test AVL tree with multiple occurances of the same value
  
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
  assert(tree.Remove(3));
  assert(aligner.GetAllocCount() == 2);
  assert(tree.GetRoot() != nullptr);
  assert(tree.GetRoot()->parent == nullptr);
  assert(IsLeftOnly(tree.GetRoot()));
  assert(IsLeaf(tree.GetRoot()->left));
  assert(ValidateParent(tree.GetRoot()->left));
  assert(tree.GetRoot()->GetValue() == 2);
  assert(tree.GetRoot()->left->GetValue() == 1);
  assert(tree.Remove(1));
  assert(aligner.GetAllocCount() == 1);
  assert(tree.GetRoot() != nullptr);
  assert(tree.GetRoot()->parent == nullptr);
  assert(IsLeaf(tree.GetRoot()));
  assert(tree.GetRoot()->GetValue() == 2);
  assert(tree.Remove(2));
  assert(aligner.GetAllocCount() == 0);
  assert(tree.GetRoot() == nullptr);
  
  // Rooted trivial case with right child.
  tree.Add(2);
  tree.Add(3);
  assert(tree.Remove(2));
  assert(tree.GetRoot() != nullptr);
  assert(tree.GetRoot()->parent == nullptr);
  assert(IsLeaf(tree.GetRoot()));
  assert(tree.GetRoot()->GetValue() == 3);
  assert(tree.Remove(3));
  assert(aligner.GetAllocCount() == 0);
  assert(tree.GetRoot() == nullptr);
  
  // Rooted trivial case with left child.
  tree.Add(2);
  tree.Add(1);
  assert(tree.Remove(2));
  assert(tree.GetRoot() != nullptr);
  assert(tree.GetRoot()->parent == nullptr);
  assert(IsLeaf(tree.GetRoot()));
  assert(tree.GetRoot()->GetValue() == 1);
  assert(tree.Remove(1));
  assert(aligner.GetAllocCount() == 0);
  assert(tree.GetRoot() == nullptr);
  
  // Unrooted trivial case with right child.
  tree.Add(2);
  tree.Add(1);
  tree.Add(3);
  tree.Add(4);
  assert(aligner.GetAllocCount() == 4);
  assert(tree.Remove(3));
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
  assert(tree.Remove(4));
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

void TestUnbalancedTrivialDeletions() {
  ScopedPass pass("AvlTree<int>::Remove() [unbalanced, trivial]");
  AvlTree<int> tree(aligner);
  
  // Leaf deletion causing single-layer right-left rebalance.
  tree.Add(2);
  tree.Add(1);
  tree.Add(4);
  tree.Add(3);
  assert(aligner.GetAllocCount() == 4);
  assert(tree.Remove(1));
  assert(aligner.GetAllocCount() == 3);
  assert(ValidateRoot(tree.GetRoot()));
  assert(IsFull(tree.GetRoot()));
  assert(IsLeaf(tree.GetRoot()->left));
  assert(IsLeaf(tree.GetRoot()->right));
  assert(ValidateParent(tree.GetRoot()->left));
  assert(ValidateParent(tree.GetRoot()->right));
  assert(tree.GetRoot()->GetValue() == 3);
  assert(tree.GetRoot()->left->GetValue() == 2);
  assert(tree.GetRoot()->right->GetValue() == 4);
  tree.Clear();
  assert(aligner.GetAllocCount() == 0);
  
  // Leaf deletion causing a right-left rotation and then a right-right
  // rotation
  int addValues[] = {8, 3, 13, 2, 6, 11, 16, 1, 4, 7, 10, 12, 15, 18, 5, 9, 14,
                     17, 19, 20};
  for (size_t i = 0; i < sizeof(addValues) / sizeof(int); ++i) {
    tree.Add(addValues[i]);
  }
  assert(aligner.GetAllocCount() == 20);
  assert(tree.Remove(1));
  assert(aligner.GetAllocCount() == 19);
  assert(ValidateRoot(tree.GetRoot()));
  assert(IsFull(tree.GetRoot()));
  assert(IsFull(tree.GetRoot()->left));
  assert(IsFull(tree.GetRoot()->right));
  // Depth 2
  assert(IsFull(tree.GetRoot()->left->left));
  assert(IsFull(tree.GetRoot()->left->right));
  assert(IsLeftOnly(tree.GetRoot()->right->left));
  assert(IsFull(tree.GetRoot()->right->right));
  // Depth 3
  assert(IsLeftOnly(tree.GetRoot()->left->left->left));
  assert(IsFull(tree.GetRoot()->left->left->right));
  assert(IsLeftOnly(tree.GetRoot()->left->right->left));
  assert(IsLeaf(tree.GetRoot()->left->right->right));
  assert(IsLeaf(tree.GetRoot()->right->left->left));
  assert(IsLeaf(tree.GetRoot()->right->right->left));
  assert(IsRightOnly(tree.GetRoot()->right->right->right));
  // Depth 4
  assert(IsLeaf(tree.GetRoot()->left->left->left->left));
  assert(IsLeaf(tree.GetRoot()->left->left->right->left));
  assert(IsLeaf(tree.GetRoot()->left->left->right->right));
  assert(IsLeaf(tree.GetRoot()->left->right->left->left));
  assert(IsLeaf(tree.GetRoot()->right->right->right->right));
  // Validate parents
  assert(ValidateParent(tree.GetRoot()->left));
  assert(ValidateParent(tree.GetRoot()->right));
  assert(ValidateParent(tree.GetRoot()->left->left));
  assert(ValidateParent(tree.GetRoot()->left->right));
  assert(ValidateParent(tree.GetRoot()->right->left));
  assert(ValidateParent(tree.GetRoot()->right->right));
  assert(ValidateParent(tree.GetRoot()->left->left->left));
  assert(ValidateParent(tree.GetRoot()->left->left->right));
  assert(ValidateParent(tree.GetRoot()->left->right->left));
  assert(ValidateParent(tree.GetRoot()->left->right->right));
  assert(ValidateParent(tree.GetRoot()->right->left->left));
  assert(ValidateParent(tree.GetRoot()->right->right->left));
  assert(ValidateParent(tree.GetRoot()->right->right->right));
  assert(ValidateParent(tree.GetRoot()->left->left->left->left));
  assert(ValidateParent(tree.GetRoot()->left->left->right->left));
  assert(ValidateParent(tree.GetRoot()->left->left->right->right));
  assert(ValidateParent(tree.GetRoot()->left->right->left->left));
  assert(ValidateParent(tree.GetRoot()->right->right->right->right));
  // Validate values
  assert(tree.GetRoot()->GetValue() == 13);
  assert(tree.GetRoot()->left->GetValue() == 8);
  assert(tree.GetRoot()->right->GetValue() == 16);
  assert(tree.GetRoot()->left->left->GetValue() == 4);
  assert(tree.GetRoot()->left->right->GetValue() == 11);
  assert(tree.GetRoot()->right->left->GetValue() == 15);
  assert(tree.GetRoot()->right->right->GetValue() == 18);
  assert(tree.GetRoot()->left->left->left->GetValue() == 3);
  assert(tree.GetRoot()->left->left->right->GetValue() == 6);
  assert(tree.GetRoot()->left->right->left->GetValue() == 10);
  assert(tree.GetRoot()->left->right->right->GetValue() == 12);
  assert(tree.GetRoot()->right->left->left->GetValue() == 14);
  assert(tree.GetRoot()->right->right->left->GetValue() == 17);
  assert(tree.GetRoot()->right->right->right->GetValue() == 19);
  assert(tree.GetRoot()->left->left->left->left->GetValue() == 2);
  assert(tree.GetRoot()->left->left->right->left->GetValue() == 5);
  assert(tree.GetRoot()->left->left->right->right->GetValue() == 7);
  assert(tree.GetRoot()->left->right->left->left->GetValue() == 9);
  assert(tree.GetRoot()->right->right->right->right->GetValue() == 20);
  tree.Clear();
  assert(aligner.GetAllocCount() == 0);
  
  // Leaf deletion causing root rebalance via right-right case which only
  // occurs when a node is deleted in a special case.
  int specialValues[] = {5, 2, 13, 1, 3, 9, 17, 4, 7, 11, 15, 19, 6, 8, 10, 12,
                         14, 16, 18, 20};
  for (size_t i = 0; i < sizeof(specialValues) / sizeof(int); ++i) {
    tree.Add(specialValues[i]);
  }
  assert(aligner.GetAllocCount() == 20);
  assert(tree.Remove(4));
  assert(aligner.GetAllocCount() == 19);
  assert(ValidateRoot(tree.GetRoot()));
  assert(IsFull(tree.GetRoot()));
  assert(IsFull(tree.GetRoot()->left));
  assert(IsFull(tree.GetRoot()->right));
  // Depth 2
  assert(IsFull(tree.GetRoot()->left->left));
  assert(IsFull(tree.GetRoot()->left->right));
  assert(IsFull(tree.GetRoot()->right->left));
  assert(IsFull(tree.GetRoot()->right->right));
  // Depth 3
  assert(IsLeaf(tree.GetRoot()->left->left->left));
  assert(IsLeaf(tree.GetRoot()->left->left->right));
  assert(IsFull(tree.GetRoot()->left->right->left));
  assert(IsFull(tree.GetRoot()->left->right->right));
  assert(IsLeaf(tree.GetRoot()->right->left->left));
  assert(IsLeaf(tree.GetRoot()->right->left->right));
  assert(IsLeaf(tree.GetRoot()->right->right->left));
  assert(IsLeaf(tree.GetRoot()->right->right->right));
  // Depth 4
  assert(IsLeaf(tree.GetRoot()->left->right->left->left));
  assert(IsLeaf(tree.GetRoot()->left->right->left->right));
  assert(IsLeaf(tree.GetRoot()->left->right->right->left));
  assert(IsLeaf(tree.GetRoot()->left->right->right->right));
  // Validate parents
  assert(ValidateParent(tree.GetRoot()->left));
  assert(ValidateParent(tree.GetRoot()->right));
  assert(ValidateParent(tree.GetRoot()->left->left));
  assert(ValidateParent(tree.GetRoot()->left->right));
  assert(ValidateParent(tree.GetRoot()->right->left));
  assert(ValidateParent(tree.GetRoot()->right->right));
  assert(ValidateParent(tree.GetRoot()->left->left->left));
  assert(ValidateParent(tree.GetRoot()->left->left->right));
  assert(ValidateParent(tree.GetRoot()->left->right->left));
  assert(ValidateParent(tree.GetRoot()->left->right->right));
  assert(ValidateParent(tree.GetRoot()->right->left->left));
  assert(ValidateParent(tree.GetRoot()->right->left->right));
  assert(ValidateParent(tree.GetRoot()->right->right->left));
  assert(ValidateParent(tree.GetRoot()->right->right->right));
  assert(ValidateParent(tree.GetRoot()->left->right->left->left));
  assert(ValidateParent(tree.GetRoot()->left->right->left->right));
  assert(ValidateParent(tree.GetRoot()->left->right->right->left));
  assert(ValidateParent(tree.GetRoot()->left->right->right->right));
  // Validate values
  assert(tree.GetRoot()->GetValue() == 13);
  assert(tree.GetRoot()->left->GetValue() == 5);
  assert(tree.GetRoot()->right->GetValue() == 17);
  assert(tree.GetRoot()->left->left->GetValue() == 2);
  assert(tree.GetRoot()->left->right->GetValue() == 9);
  assert(tree.GetRoot()->right->left->GetValue() == 15);
  assert(tree.GetRoot()->right->right->GetValue() == 19);
  assert(tree.GetRoot()->left->left->left->GetValue() == 1);
  assert(tree.GetRoot()->left->left->right->GetValue() == 3);
  assert(tree.GetRoot()->left->right->left->GetValue() == 7);
  assert(tree.GetRoot()->left->right->right->GetValue() == 11);
  assert(tree.GetRoot()->right->left->left->GetValue() == 14);
  assert(tree.GetRoot()->right->left->right->GetValue() == 16);
  assert(tree.GetRoot()->right->right->left->GetValue() == 18);
  assert(tree.GetRoot()->right->right->right->GetValue() == 20);
  assert(tree.GetRoot()->left->right->left->left->GetValue() == 6);
  assert(tree.GetRoot()->left->right->left->right->GetValue() == 8);
  assert(tree.GetRoot()->left->right->right->left->GetValue() == 10);
  assert(tree.GetRoot()->left->right->right->right->GetValue() == 12);
  tree.Clear();
  assert(aligner.GetAllocCount() == 0);
  
  // Test the mirror of the above case
  int specialValuesMirror[] = {16, 8, 19, 4, 12, 18, 20, 2, 6, 10, 14, 17, 1,
                               3, 5, 7, 9, 11, 13, 15};
  for (size_t i = 0; i < sizeof(specialValuesMirror) / sizeof(int); ++i) {
    tree.Add(specialValuesMirror[i]);
  }
  assert(aligner.GetAllocCount() == 20);
  assert(tree.Remove(17));
  assert(aligner.GetAllocCount() == 19);
  assert(ValidateRoot(tree.GetRoot()));
  assert(IsFull(tree.GetRoot()));
  assert(IsFull(tree.GetRoot()->right));
  assert(IsFull(tree.GetRoot()->left));
  // Depth 2
  assert(IsFull(tree.GetRoot()->right->right));
  assert(IsFull(tree.GetRoot()->right->left));
  assert(IsFull(tree.GetRoot()->left->right));
  assert(IsFull(tree.GetRoot()->left->left));
  // Depth 3
  assert(IsLeaf(tree.GetRoot()->right->right->right));
  assert(IsLeaf(tree.GetRoot()->right->right->left));
  assert(IsFull(tree.GetRoot()->right->left->right));
  assert(IsFull(tree.GetRoot()->right->left->left));
  assert(IsLeaf(tree.GetRoot()->left->right->right));
  assert(IsLeaf(tree.GetRoot()->left->right->left));
  assert(IsLeaf(tree.GetRoot()->left->left->right));
  assert(IsLeaf(tree.GetRoot()->left->left->left));
  // Depth 4
  assert(IsLeaf(tree.GetRoot()->right->left->right->right));
  assert(IsLeaf(tree.GetRoot()->right->left->right->left));
  assert(IsLeaf(tree.GetRoot()->right->left->left->right));
  assert(IsLeaf(tree.GetRoot()->right->left->left->left));
  // Validate parents
  assert(ValidateParent(tree.GetRoot()->right));
  assert(ValidateParent(tree.GetRoot()->left));
  assert(ValidateParent(tree.GetRoot()->right->right));
  assert(ValidateParent(tree.GetRoot()->right->left));
  assert(ValidateParent(tree.GetRoot()->left->right));
  assert(ValidateParent(tree.GetRoot()->left->left));
  assert(ValidateParent(tree.GetRoot()->right->right->right));
  assert(ValidateParent(tree.GetRoot()->right->right->left));
  assert(ValidateParent(tree.GetRoot()->right->left->right));
  assert(ValidateParent(tree.GetRoot()->right->left->left));
  assert(ValidateParent(tree.GetRoot()->left->right->right));
  assert(ValidateParent(tree.GetRoot()->left->right->left));
  assert(ValidateParent(tree.GetRoot()->left->left->right));
  assert(ValidateParent(tree.GetRoot()->left->left->left));
  assert(ValidateParent(tree.GetRoot()->right->left->right->right));
  assert(ValidateParent(tree.GetRoot()->right->left->right->left));
  assert(ValidateParent(tree.GetRoot()->right->left->left->right));
  assert(ValidateParent(tree.GetRoot()->right->left->left->left));
  // Validate values
  assert(tree.GetRoot()->GetValue() == 8);
  assert(tree.GetRoot()->left->GetValue() == 4);
  assert(tree.GetRoot()->right->GetValue() == 16);
  // Depth 2
  assert(tree.GetRoot()->left->left->GetValue() == 2);
  assert(tree.GetRoot()->left->right->GetValue() == 6);
  assert(tree.GetRoot()->right->left->GetValue() == 12);
  assert(tree.GetRoot()->right->right->GetValue() == 19);
  // Depth 3
  assert(tree.GetRoot()->left->left->left->GetValue() == 1);
  assert(tree.GetRoot()->left->left->right->GetValue() == 3);
  assert(tree.GetRoot()->left->right->left->GetValue() == 5);
  assert(tree.GetRoot()->left->right->right->GetValue() == 7);
  assert(tree.GetRoot()->right->left->left->GetValue() == 10);
  assert(tree.GetRoot()->right->left->right->GetValue() == 14);
  assert(tree.GetRoot()->right->right->left->GetValue() == 18);
  assert(tree.GetRoot()->right->right->right->GetValue() == 20);
  // Depth 4
  assert(tree.GetRoot()->right->left->left->left->GetValue() == 9);
  assert(tree.GetRoot()->right->left->left->right->GetValue() == 11);
  assert(tree.GetRoot()->right->left->right->left->GetValue() == 13);
  assert(tree.GetRoot()->right->left->right->right->GetValue() == 15);
}

void TestBalancedNontrivialDeletions() {
  ScopedPass pass("AvlTree<int>::Remove() [balanced, non-trivial]");
  AvlTree<int> tree(aligner);
  
  // No left subnode of in-order predecessor; removing root
  tree.Add(4);
  tree.Add(2);
  tree.Add(6);
  tree.Add(1);
  tree.Add(3);
  tree.Add(5);
  tree.Add(7);
  assert(aligner.GetAllocCount() == 7);
  assert(tree.Remove(4));
  assert(aligner.GetAllocCount() == 6);
  assert(ValidateRoot(tree.GetRoot()));
  assert(IsFull(tree.GetRoot()));
  assert(IsLeftOnly(tree.GetRoot()->left));
  assert(IsFull(tree.GetRoot()->right));
  assert(IsLeaf(tree.GetRoot()->left->left));
  assert(IsLeaf(tree.GetRoot()->right->left));
  assert(IsLeaf(tree.GetRoot()->right->right));
  // Validate parents
  assert(ValidateParent(tree.GetRoot()->left));
  assert(ValidateParent(tree.GetRoot()->right));
  assert(ValidateParent(tree.GetRoot()->left->left));
  assert(ValidateParent(tree.GetRoot()->right->left));
  assert(ValidateParent(tree.GetRoot()->right->right));
  // Validate values
  assert(tree.GetRoot()->GetValue() == 3);
  assert(tree.GetRoot()->left->GetValue() == 2);
  assert(tree.GetRoot()->right->GetValue() == 6);
  assert(tree.GetRoot()->left->left->GetValue() == 1);
  assert(tree.GetRoot()->right->left->GetValue() == 5);
  assert(tree.GetRoot()->right->right->GetValue() == 7);
  tree.Clear();
  assert(aligner.GetAllocCount() == 0);
  
  // No left subnode of in-order predecessor; removing child of root
  tree.Add(4);
  tree.Add(2);
  tree.Add(6);
  tree.Add(1);
  tree.Add(3);
  tree.Add(5);
  tree.Add(7);
  assert(aligner.GetAllocCount() == 7);
  assert(tree.Remove(2));
  assert(aligner.GetAllocCount() == 6);
  assert(ValidateRoot(tree.GetRoot()));
  assert(IsFull(tree.GetRoot()));
  assert(IsRightOnly(tree.GetRoot()->left));
  assert(IsFull(tree.GetRoot()->right));
  assert(IsLeaf(tree.GetRoot()->left->right));
  assert(IsLeaf(tree.GetRoot()->right->left));
  assert(IsLeaf(tree.GetRoot()->right->right));
  // Validate parents
  assert(ValidateParent(tree.GetRoot()->left));
  assert(ValidateParent(tree.GetRoot()->right));
  assert(ValidateParent(tree.GetRoot()->left->right));
  assert(ValidateParent(tree.GetRoot()->right->left));
  assert(ValidateParent(tree.GetRoot()->right->right));
  // Validate values
  assert(tree.GetRoot()->GetValue() == 4);
  assert(tree.GetRoot()->left->GetValue() == 1);
  assert(tree.GetRoot()->right->GetValue() == 6);
  assert(tree.GetRoot()->left->right->GetValue() == 3);
  assert(tree.GetRoot()->right->left->GetValue() == 5);
  assert(tree.GetRoot()->right->right->GetValue() == 7);
  tree.Clear();
  assert(aligner.GetAllocCount() == 0);
  
  // Non-null left subnode of in-order predecessor; removing root
  tree.Add(5);
  tree.Add(2);
  tree.Add(7);
  tree.Add(1);
  tree.Add(4);
  tree.Add(6);
  tree.Add(8);
  tree.Add(3);
  assert(aligner.GetAllocCount() == 8);
  assert(tree.Remove(5));
  assert(aligner.GetAllocCount() == 7);
  assert(ValidateRoot(tree.GetRoot()));
  assert(IsFull(tree.GetRoot()));
  assert(IsFull(tree.GetRoot()->left));
  assert(IsFull(tree.GetRoot()->right));
  assert(IsLeaf(tree.GetRoot()->left->left));
  assert(IsLeaf(tree.GetRoot()->left->right));
  assert(IsLeaf(tree.GetRoot()->right->left));
  assert(IsLeaf(tree.GetRoot()->right->right));
  // Validate parents
  assert(ValidateParent(tree.GetRoot()->left));
  assert(ValidateParent(tree.GetRoot()->right));
  assert(ValidateParent(tree.GetRoot()->left->left));
  assert(ValidateParent(tree.GetRoot()->left->right));
  assert(ValidateParent(tree.GetRoot()->right->left));
  assert(ValidateParent(tree.GetRoot()->right->right));
  // Validate values
  assert(tree.GetRoot()->GetValue() == 4);
  assert(tree.GetRoot()->left->GetValue() == 2);
  assert(tree.GetRoot()->right->GetValue() == 7);
  assert(tree.GetRoot()->left->left->GetValue() == 1);
  assert(tree.GetRoot()->left->right->GetValue() == 3);
  assert(tree.GetRoot()->right->left->GetValue() == 6);
  assert(tree.GetRoot()->right->right->GetValue() == 8);
  tree.Clear();
  assert(aligner.GetAllocCount() == 0);
  
  // Non-null left subnode of in-order predecessor; removing child of root
  tree.Add(5);
  tree.Add(3);
  tree.Add(7);
  tree.Add(1);
  tree.Add(4);
  tree.Add(6);
  tree.Add(8);
  tree.Add(2);
  assert(aligner.GetAllocCount() == 8);
  assert(tree.Remove(3));
  assert(aligner.GetAllocCount() == 7);
  assert(ValidateRoot(tree.GetRoot()));
  assert(IsFull(tree.GetRoot()));
  assert(IsFull(tree.GetRoot()->left));
  assert(IsFull(tree.GetRoot()->right));
  assert(IsLeaf(tree.GetRoot()->left->left));
  assert(IsLeaf(tree.GetRoot()->left->right));
  assert(IsLeaf(tree.GetRoot()->right->left));
  assert(IsLeaf(tree.GetRoot()->right->right));
  // Validate parents
  assert(ValidateParent(tree.GetRoot()->left));
  assert(ValidateParent(tree.GetRoot()->right));
  assert(ValidateParent(tree.GetRoot()->left->left));
  assert(ValidateParent(tree.GetRoot()->left->right));
  assert(ValidateParent(tree.GetRoot()->right->left));
  assert(ValidateParent(tree.GetRoot()->right->right));
  // Validate values
  assert(tree.GetRoot()->GetValue() == 5);
  assert(tree.GetRoot()->left->GetValue() == 2);
  assert(tree.GetRoot()->right->GetValue() == 7);
  assert(tree.GetRoot()->left->left->GetValue() == 1);
  assert(tree.GetRoot()->left->right->GetValue() == 4);
  assert(tree.GetRoot()->right->left->GetValue() == 6);
  assert(tree.GetRoot()->right->right->GetValue() == 8);
  tree.Clear();
  assert(aligner.GetAllocCount() == 0);
}

void TestRandomModifications() {
  ScopedPass pass("AvlTree<int>::[Remove/Add]() [pre-randomized]");
  AvlTree<int> tree(aligner);
  
  int modifications[] = {2, 1, 60, 78, 52, 47, 13, 19, 30, 9, 84, 23, 66, 97,
      27, 25, 15, 37, 90, 65, 61, 35, 31, 18, 100, 39, 91, 12, 76, 92, -13,
      -76, -12, -37, -66, -23, -2, -91, -65, -60, -97, -25, -15, -31, -18,
      -35, -61, -30, -9, -27, -78, -1, -52, -84, -100, -90, -19, -92, -39,
      -47, 44, 10, 31, 7, 99, 32, 70, 47, 89, 90, 18, 76, 82, 77, 2, 33, 37,
      20, 16, 91, 69, 84, 93, 53, 6, 19, 59, 39, -37, -6, -32, -10, -91, -76,
      -53, -19, -84, -77, -59, -99, -33, -44, -93, -39, -47, -89, -69, -16,
      -70, -82, -20, -2, -31, -90, -7, 15, 64, 61, 25, 55, 53, 49, 17, 45, 46,
      1, 20, 91, 78, 83, -46, -64, -15, -18, -25, -45, -83, -17, -91, -53,
      -20, -61, -1, -78, 80, 98, 15, 100, 5, 90, 81, 39, 48, 76, 1, 94, -76,
      -98, -94, -55, -15, -1, -100, -5, -81, -90, -39, -80, -49, 20, 71, 35,
      86, 16, 44, 54, 68, 96, 63, 85, 95, 1, 84, 17, 73, 67, 39, 22, 94, 6, 7,
      47, 53, 42, 81, 38, 12, 23, 59, 91, 18, 51, 56, 98, 92, 3, 25, 72, 32,
      19, 4, 57, 76, 88, 74, 28, 52, 5, 97, -59, -94, -42, -63, -48, -72, -16,
      -76, -74, -3, -73, -35, -17, -97, -25, -19, -98, -38, -68, -12, -18, 97,
      76, 60, 17, 50, 78, 27, 2, 36, 14, 61, 24, 70, 55, 80, 94, 38, 46, 16,
      65, 66, 35, 34, 64, 98, 83, 73, 77, 69, 41, 31, 59, 29, 90, 26, 18, 43,
      79, 74, 42, 37, 49, 21, 9, 8, 58, 12, 25, 75, 72, 68, 33, 11, 10, 19,
      30, 48, 13, 45, 3, 93, 15, 62, -23, -72, -67, -92, -24, -14, -20, -6,
      -42, -5, -25, -97, -80, -49, -51, -11, -2, -85, -73, -52, -54, -43, -71,
      -90, -53, -96, -9, -94, -60, -79, -30, -65, -19, -68, -4, -48, -57, -61,
      -55, -69, -70, -44, -16, -3, -34, -74, -91, -64, -56, -75, -98, -26,
      -32, -13, -45, -35, -50, -7, -29, -93, -77, -15, -58, -76, -8, -46, -83,
      -39, -10, -31, -38, -81, -66, -22, -78, -21, -1, -33, -62, -88, -37,
      -41, -47, -84, -27, -36, -86, -28, -95, -18, 49, 52, 46, 45, 72, 48, 10,
      58, 95, 74, 6, 37, 94, 83, 70, 31, 21, 77, 67, 11, 92, 32, 60, 84, 65,
      55, 97, 66, 86, 13, 96, 38, 40, 88, 91, 22, 2, 64, 28, 68, 54, 79, 19,
      85, 7, 34, 15, 23, 57, 56, 80, 76, 26, 42, 39, 36, 61, 81, 29, 99, 90,
      50, 44, 41, 27, 33, 25, 24, 30, 4, 53, 5, 78, 16, 35, 89, 47, 93, 1, 14,
      87, 98, 82, 73, 63, 8, 71, 100, 3, 9, 51, 75, 43, 62, 20, 18, -34, -55,
      -56, -67, -40, -63, -54, -75, -1, -20, -17, -96, -36, -27, -11, -14, -8,
      -86, -68, -58, -51, -100, -59, -30, -76, -60, -43, -23, -91, -97, -44,
      -32, -88, -15, -41, -99, -95, -61, -62, -98, -35, -19, -92, 44, 76, 36,
      75, 17, 61, 41, 100, 88, 54, 32, 19, 55, 67, 56, 51, 60, 11, 97, 34, 23,
      1, 15, 98, 43, 69, 95, 27, 8, 68, 91, 92, 58, 86, 30, 63, 40, 59, 14,
      96, 20, 35, 99, -100, -91, -75, -11, -18, -89, -88, -16, -39, -78, -93,
      -73, -63, -94, -33, -72, -61, -38, -48, -69, -82, -96, -41, -34, -7,
      -43, -4, -37, -20, -80, -40, -25, -6, -64, -1, -99, -66, -86, -8, -49,
      -50, -95, -22, -9, -12, -56, -52, -59, -44, -23, -17, -46, -47, -29, -2,
      -26, -24, -87, -84, -15, -45, -74, -36, -58, -67, -85, -68, -81, -92,
      -53, -19, -60, -30, -27, -35, -70, -83, -65, -55, -54, -14, -51, -32,
      -77, -21, -76, -97, -3, -5, -71, 45, 35, 43, 56, 34, 26, 61, 27, 88, 6,
      1, 62, 47, 71, 21, 4, 14, 54, 89, 82, 67, 29, 68, 2, 36, 91, 8, 7, 53,
      38, 32, 20, 60, 84, 5, 63, 37, 80, 52, 44, 97, 93, 100, 95, 83, 33, 9,
      66};
  int inside[] = {10, 31, 13, 28, 79, 57, 42, 90, 98, 45, 35, 43, 56, 34, 26,
                  61, 27, 88, 6, 1, 62, 47, 71, 21, 4, 14, 54, 89, 82, 67, 29,
                  68, 2, 36, 91, 8, 7, 53, 38, 32, 20, 60, 84, 5, 63, 37, 80,
                  52, 44, 97, 93, 100, 95, 83, 33, 9, 66};
  int outside[] = {75, 11, 18, 16, 39, 78, 73, 94, 72, 48, 69, 96, 41, 40, 25,
                   64, 99, 86, 49, 50, 22, 12, 59, 23, 17, 46, 24, 87, 15, 74,
                   58, 85, 81, 92, 19, 30, 70, 65, 55, 51, 77, 76, 3};
  for (size_t i = 0; i < sizeof(modifications) / sizeof(int); ++i) {
    int val = modifications[i];
    if (val > 0) {
      assert(!tree.Contains(val));
      assert(tree.Add(val));
    } else {
      assert(tree.Remove(-val));
    }
    int ignored;
    assert(ValidateBalance(tree.GetRoot(), ignored));
  }
  for (size_t i = 0; i < sizeof(inside) / sizeof(int); ++i) {
    assert(tree.Contains(inside[i]));
  }
  for (size_t i = 0; i < sizeof(outside) / sizeof(int); ++i) {
    assert(!tree.Contains(outside[i]));
  }
}

void TestFindMethods() {
  ScopedPass pass("AvlTree<int>::[Find*]()");
  AvlTree<int> tree(aligner);
  
  tree.Add(10);
  tree.Add(6);
  tree.Add(16);
  tree.Add(4);
  tree.Add(8);
  tree.Add(12);
  tree.Add(18);
  tree.Add(2);
  tree.Add(14);
  
  int result;
  
  // Values inside the dataset
  assert(tree.FindGT(result, 15));
  assert(result == 16);
  assert(tree.FindGE(result, 15));
  assert(result == 16);
  assert(tree.FindGT(result, 14));
  assert(result == 16);
  assert(tree.FindGE(result, 14));
  assert(result == 14);
  assert(tree.FindLT(result, 7));
  assert(result == 6);
  assert(tree.FindLE(result, 7));
  assert(result == 6);
  assert(tree.FindLT(result, 8));
  assert(result == 6);
  assert(tree.FindLE(result, 8));
  assert(result == 8);
  
  // Values outside of the range of the dataset
  assert(!tree.FindLT(result, 2));
  assert(tree.FindLE(result, 2));
  assert(result == 2);
  assert(!tree.FindLE(result, 1));
  assert(!tree.FindGT(result, 18));
  assert(tree.FindGE(result, 18));
  assert(result == 18);
  assert(!tree.FindGE(result, 19));
  
  // Find and remove functionality
  assert(tree.FindLE(result, 6, true));
  assert(result == 6);
  assert(!tree.Contains(6));
  assert(tree.FindLT(result, 5, true));
  assert(result == 4);
  assert(!tree.Contains(4));
  assert(tree.FindGT(result, 1, true));
  assert(result == 2);
  assert(!tree.Contains(2));
  assert(tree.FindGE(result, 8, true));
  assert(result == 8);
  assert(!tree.Contains(8));
}

void TestSearchFunction() {
  ScopedPass pass("AvlTree<int>::Search()");
  AvlTree<int> tree(aligner);
  
  struct SearchFunc : public AvlTree<int>::Query {
    virtual int DirectionFromNode(const int & test) const {
      if (value > test) return 1;
      else if (value < test) return -1;
      else return 0;
    }
    
    int value;
  };
  
  tree.Add(10);
  tree.Add(6);
  tree.Add(16);
  tree.Add(4);
  tree.Add(8);
  tree.Add(12);
  tree.Add(18);
  tree.Add(2);
  tree.Add(14);
  
  SearchFunc func;
  int result;
  
  // Basic searches
  func.value = 2;
  assert(tree.Search(result, func));
  assert(result == 2);
  func.value = 1;
  assert(!tree.Search(result, func));
  func.value = 18;
  assert(tree.Search(result, func));
  assert(result == 18);
  func.value = 19;
  assert(!tree.Search(result, func));
  func.value = 10;
  assert(tree.Search(result, func));
  assert(result == 10);
  func.value = 9;
  assert(!tree.Search(result, func));
  
  // Deleting searches
  func.value = 2;
  assert(tree.Search(result, func, true));
  assert(result == 2);
  assert(!tree.Contains(2));
  assert(!tree.Search(result, func, true));
  assert(!tree.Search(result, func));
  func.value = 18;
  assert(tree.Search(result, func, true));
  assert(result == 18);
  assert(!tree.Contains(18));
  assert(!tree.Search(result, func, true));
  assert(!tree.Search(result, func));
  func.value = 10;
  assert(tree.Search(result, func, true));
  assert(result == 10);
  assert(!tree.Contains(10));
  assert(!tree.Search(result, func, true));
  assert(!tree.Search(result, func));
}

void TestEnumerator() {
  struct RollingEnumerator : public AvlTree<int>::EnumerateCallback {
    int values[100];
    int idx = 0;
    
    virtual bool Yield(const int & value) {
      assert(idx < 100);
      values[idx++] = value;
      return true;
    }
  };
  
  struct StoppingEnumerator : public RollingEnumerator {
    bool stopped = false;
    int cutoff;
    
    virtual bool Yield(const int & value) {
      assert(!stopped);
      RollingEnumerator::Yield(value);
      if (idx == cutoff) {
        return false;
      } else {
        return true;
      }
    }
  };
  
  ScopedPass pass("AvlTree<int>::Enumerate()");
  AvlTree<int> tree(aligner);
  
  int numbers[] = {97, 28, 70, 100, 12, 89, 62, 67, 16, 61, 36, 45, 87, 78,
      74, 95, 80, 53, 6, 65, 40, 50, 92, 47, 64, 76, 96, 99, 29, 46, 30, 94,
      84, 71, 54, 31, 73, 33, 69, 43, 72, 18, 63, 83, 3, 52, 90, 13, 68, 34,
      32, 9, 55, 15, 35, 17, 86, 91, 81, 38, 75, 37, 77, 41, 60, 79, 8, 4,
      20, 58, 57, 93, 21, 2, 88, 27, 19, 25, 7, 48, 26, 5, 85, 39, 23, 14, 1,
      10, 11, 98, 49, 24, 82, 42, 66, 51, 59, 22, 56, 44};
  for (int i = 0; i < 100; ++i) {
    tree.Add(numbers[i]);
  }
  RollingEnumerator enum1;
  StoppingEnumerator enum2;
  StoppingEnumerator enum3;
  enum2.cutoff = 20;
  enum3.cutoff = 100;
  assert(tree.Enumerate(enum1));
  assert(enum1.idx == 100);
  for (int i = 0; i < 100; ++i) {
    assert(enum1.values[i] == i + 1);
  }
  assert(!tree.Enumerate(enum2));
  assert(enum2.idx == 20);
  for (int i = 0; i < 20; ++i) {
    assert(enum2.values[i] == i + 1);
  }
  assert(!tree.Enumerate(enum3));
  assert(enum3.idx == 100);
  for (int i = 0; i < 100; ++i) {
    assert(enum3.values[i] == i + 1);
  }
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

bool ValidateRoot(const AvlNode<int> * node) {
  return node != nullptr && node->parent == nullptr;
}

bool ValidateBalance(const AvlNode<int> * node, int & depthOut) {
  if (!node) {
    depthOut = 0;
    return true;
  }
  if (!ValidateParent(node)) {
    return false;
  }
  int d1, d2;
  bool res1 = ValidateBalance(node->left, d1);
  bool res2 = ValidateBalance(node->right, d2);
  if (!res1 || !res2) {
    return false;
  }
  if (d1 - d2 > 1 || d2 - d1 > 1) {
    return false;
  }
  depthOut = 1 + ansa::Max(d1, d2);
  if (depthOut != node->depth + 1) {
    return false;
  }
  return true;
}
