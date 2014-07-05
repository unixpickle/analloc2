#include "bbtree.hpp"

namespace ANAlloc {

size_t BBTree::MemorySize(int depth) {
  uint64_t bitCount = TreeSizeAtDepth(depth);
  if (bitCount & 7) return (1UL << bitCount) / 8 + 1;
  return (1UL << bitCount) / 8;
}

BBTree::BBTree() {
}

BBTree::BBTree(int _depth, uint8_t * bmMemory)
  : bitmap(bmMemory, TreeSizeAtDepth(_depth)), depth(_depth) {
  WriteNode(Path::RootPath(), _depth);
}

BBTree::BBTree(const BBTree & tree) : bitmap(tree.bitmap), depth(tree.depth) {
}

BBTree & BBTree::operator=(const BBTree & tree) {
  bitmap = tree.bitmap;
  depth = tree.depth;
  return *this;
}

int BBTree::GetDepth() {
  return depth;
}

void BBTree::SetType(Path path, NodeType type) {
  // TODO: this
}

NodeType BBTree::GetType(Path path) {
  // TODO: this
}

bool BBTree::FindFree(int depth, Path & path) {
  // TODO: this
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

uint64_t BBTree::FieldSizeAtDepth(int _depth) {
  // log table probably not the best, but it's fast
  uint64_t numberLogs[] = {
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

}
