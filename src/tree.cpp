#include "tree.hpp"

namespace ANAlloc {

void Tree::Free(Path p) {
  NodeType type = GetType(p);
  if (type != NodeTypeFree) {
    if (type == NodeTypeContainer) {
      Free(p.Left());
      Free(p.Right());
    }
    SetType(p, NodeTypeFree);
  }
}

void Tree::FreeAll() {
  Free(Path(0, 0));
}

}
