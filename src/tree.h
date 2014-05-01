#ifndef __ANALLOC2_TREE_H__
#define __ANALLOC2_TREE_H__

#include <cstddef>
#include <cstdint>

namespace ANAlloc {

/**
 * This type represents the path to a node in a binary tree.
 */
typedef uintptr_t Path;

/**
 * Returns the parent path for a path. The root path has no siblings, so this
 * will return the root path.
 */
Path PathParent(Path p);

/**
 * Returns the sibling path for a path. The root path has no siblings, so
 * this will return the root path.
 */
Path PathSibling(Path p);

/**
 * Returns the right sub-path for a path.
 */
Path PathRight(Path p);

/**
 * Returns the left sub-path for a path.
 */
Path PathLeft(Path p);

/**
 * Returns the depth of a path. The depth 0 is the root node, and then it goes
 * up from there.
 */
int PathDepth(Path p);

/**
 * If the depth of the path is n, this is a number from 0 to 2^n-1 inclusive.
 * This is the index of the node at its corresponding depth. The indexes start
 * at 0 from the left.
 */
uintptr_t PathIndex(Path p);

/**
 * This is essentially an interface for a "Tree"
 */
class Tree {
public:
  typedef enum {
    NodeTypeFree,
    NodeTypeData,
    NodeTypeContainer
  } NodeType;
  
  int Depth() {
    return 0;
  }
  
  void SetType(Path path, NodeType type) {
    (void)path;
    (void)type;
  }
  
  NodeType GetType(Path path) {
    (void)path;
    return NodeTypeFree;
  }
  
  /**
   * Find a free node of at *least* a depth `depth`.
   */
  bool FindFree(int depth, Path & path) {
    return 0;
  }
  
};

}

#endif
