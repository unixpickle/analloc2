#include "bbtree.hpp"
#include <cassert>

namespace ANAlloc {

size_t BBTree::MemorySize(int depth) {
  uint64_t bitCount = TreeSizeAtDepth(depth);
  if (bitCount & 7) return bitCount / 8 + 1;
  return bitCount / 8;
}

BBTree::BBTree() {
}

BBTree::BBTree(int _depth, uint8_t * bmMemory)
  : bitmap(bmMemory, TreeSizeAtDepth(_depth)), depth(_depth) {
#ifndef ANALLOC_BBTREE_DONT_CACHE_PREFIXES
  for (int i = 0; i < depth; i++) {
    prefixSizes[i] = CalculatePrefixSize(i);
  }
#endif
  WriteNode(Path::Root(), depth);
}

BBTree::BBTree(const BBTree & tree) : bitmap(tree.bitmap), depth(tree.depth) {
#ifndef ANALLOC_BBTREE_DONT_CACHE_PREFIXES
  for (int i = 0; i < depth; i++) {
    prefixSizes[i] = CalculatePrefixSize(i);
  }
#endif
}

BBTree & BBTree::operator=(const BBTree & tree) {
  bitmap = tree.bitmap;
  depth = tree.depth;
#ifndef ANALLOC_BBTREE_DONT_CACHE_PREFIXES
  for (int i = 0; i < depth; i++) {
    prefixSizes[i] = CalculatePrefixSize(i);
  }
#endif
  return *this;
}

int BBTree::GetDepth() {
  return depth;
}

void BBTree::SetType(Path path, NodeType type) {
  int oldValue = ReadNode(path);
  int newValue = 0;
  
  if (type == NodeTypeFree) {
    newValue = depth - path.GetDepth();
  } else if (type == NodeTypeData) {
    newValue = 0;
    if (path.GetDepth() < depth - 1) {
      // if a node's value is 0, it could either be a container with two full
      // data nodes, or it could be a data node with two "free" subnodes.
      WriteNode(path.Left(), depth - path.GetDepth() - 1);
      WriteNode(path.Right(), depth - path.GetDepth() - 1);
    }
  } else {
    assert(path.GetDepth() < depth - 1);
    newValue = depth - path.GetDepth() - 1;
    
    // subnodes should automatically be free'd
    WriteNode(path.Left(), depth - path.GetDepth() - 1);
    WriteNode(path.Right(), depth - path.GetDepth() - 1);
  }
  if (newValue != oldValue) {
    WriteNode(path, newValue);
    UpdateParents(path, newValue);
  }
}

NodeType BBTree::GetType(Path path) {
  int value = ReadNode(path);
  if (!value) {
    if (path.GetDepth() == depth - 1) {
      return NodeTypeData;
    }
    if (IsFree(path.Left()) && IsFree(path.Right())) {
      return NodeTypeData;
    }
    return NodeTypeContainer;
  }
  if (value == depth - path.GetDepth()) {
    return NodeTypeFree;
  }
  return NodeTypeContainer;
}

bool BBTree::FindFree(int _depth, Path & path) {
  int minimumNode = depth - _depth;
  assert(minimumNode > 0);
  
  path = Path::Root();
  int pathValue = ReadNode(path);
  
  // this is the part that's O(log(N))
  int nodeMaximum = depth;
  while (1) {
    if (pathValue < minimumNode) return false;
    if (pathValue == nodeMaximum) return true;
    
    --nodeMaximum;
    assert(nodeMaximum != 0);
    
    Path left = path.Left();
    pathValue = ReadNode(left);
    if (pathValue >= minimumNode) {
      path = left;
    } else {
      path = path.Right();
      pathValue = ReadNode(path);
    }
  }
}

bool BBTree::FindAligned(int _depth, int align, Path & path) {
  // TODO: profile to see if this O(log(n)) optimization is ever worth it
  if (FindFree(align > depth ? depth : align, path)) return true;
  return RecursiveFindAligned(_depth, align, Path::Root(), path);
}

void BBTree::Free(Path path) {
  if (IsFree(path)) return;
  
  // this is technically not allowed by the specification, but in our tree
  // implementation it is fine.
  SetType(path, NodeTypeFree);
}

bool BBTree::IsFree(Path path) {
  return ReadNode(path) == depth - path.GetDepth();
}

// protected //

uint64_t BBTree::TreeSizeAtDepth(int depth) {
  // size table is the best we're going to get
  uint64_t treeSizes[] = {
    0UL, 1UL, 4UL, 10UL, 23UL, 49UL, 101UL, 205UL, 414UL, 832UL, 1668UL,
    3340UL, 6684UL, 13372UL, 26748UL, 53500UL, 107005UL, 214015UL, 428035UL,
    856075UL, 1712155UL, 3424315UL, 6848635UL, 13697275UL, 27394555UL,
    54789115UL, 109578235UL, 219156475UL, 438312955UL, 876625915UL,
    1753251835UL, 3506503675UL, 7013007356UL, 14026014718UL, 28052029442UL,
    56104058890UL, 112208117786UL, 224416235578UL, 448832471162UL,
    897664942330UL, 1795329884666UL, 3590659769338UL, 7181319538682UL,
    14362639077370UL, 28725278154746UL, 57450556309498UL, 114901112619002UL,
    229802225238010UL, 459604450476026UL, 919208900952058UL,
    1838417801904122UL, 3676835603808250UL, 7353671207616506UL,
    14707342415233018UL, 29414684830466042UL, 58829369660932090UL,
    117658739321864186UL, 235317478643728378UL, 470634957287456762UL,
    941269914574913530UL, 1882539829149827066UL, 3765079658299654138UL,
    7530159316599308282UL, 15060318633198616570UL
  };
  return treeSizes[depth];
}

int BBTree::FieldSizeAtDepth(int _depth) {
  // log table probably not the best, but it's fast
  int numberLogs[] = {
    0, 0,
    1,
    2, 2,
    3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6
  };
  return numberLogs[depth - _depth + 1];
}

uint64_t BBTree::CalculatePrefixSize(int _depth) {
  uint64_t result = 0;
  for (int i = 0; i < _depth; i++) {
    result += (uint64_t)FieldSizeAtDepth(i) * (1UL << i);
  }
  return result;
}

uint64_t BBTree::GetPrefixSize(int _depth) {
#ifndef ANALLOC_BBTREE_DONT_CACHE_PREFIXES
  return prefixSizes[_depth];
#else
  return CalculatePrefixSize(_depth);
#endif
}


int BBTree::ReadNode(Path p) {
  uint64_t fieldSize = FieldSizeAtDepth(p.GetDepth());
  uint64_t offset = GetPrefixSize(p.GetDepth()) + fieldSize * p.GetIndex();
  return bitmap.GetMultibit(offset, fieldSize);
}

void BBTree::WriteNode(Path p, int value) {
  uint64_t fieldSize = FieldSizeAtDepth(p.GetDepth());
  uint64_t offset = GetPrefixSize(p.GetDepth()) + fieldSize * p.GetIndex();
  bitmap.SetMultibit(offset, fieldSize, (uint64_t)value);
}

void BBTree::UpdateParents(Path p, int pValue) {
  Path currentPath = p;
  int currentValue = pValue;
  
  while (currentPath.GetDepth()) {
    int siblingValue = ReadNode(currentPath.Sibling());
    int newParentValue = siblingValue > currentValue
      ? siblingValue : currentValue;
    
    currentPath = currentPath.Parent();
    currentValue = ReadNode(currentPath);
    if (newParentValue == currentValue) {
      break;
    } else {
      currentValue = newParentValue;
      WriteNode(currentPath, currentValue);
    }
  }
}

bool BBTree::RecursiveFindAligned(int _depth, int align, Path path,
                                  Path & pathOut) {
  int pathValue = ReadNode(path);
  if (pathValue < depth - _depth) return false;
  if (pathValue == depth - path.GetDepth()) {
    pathOut = path;
    return true;
  }
  
  if (RecursiveFindAligned(_depth, align, path.Left(), pathOut)) {
    return true;
  } else if (path.GetDepth() < align) {
    return RecursiveFindAligned(_depth, align, path.Right(), pathOut);
  } else {
    return false;
  }
}

}
