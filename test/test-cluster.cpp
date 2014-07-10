#include "../src/fixed-cluster.hpp"
#include "../src/tree/btree.hpp"
#include "../src/tree/bbtree.hpp"
#include "scoped-pass.hpp"

using namespace ANAlloc;

template <class T>
void TestReserveNormal(const char * treeName);

template <class T>
void TestOperations(const char * treeName);

int main() {
  TestReserveNormal<BTree>("BTree");
  TestReserveNormal<BBTree>("BBTree");
  TestOperations<BTree>("BTree");
  TestOperations<BBTree>("BBTree");
  return 0;
}

template <class T>
void TestReserveNormal(const char * treeName) {
  ScopedPass pass("Cluster::Reserve() [normal, ", treeName, "]");
  
  uint8_t * treeData1 = new uint8_t[T::MemorySize(10)];
  uint8_t * treeData2 = new uint8_t[T::MemorySize(10)];
  uint8_t * treeData3 = new uint8_t[T::MemorySize(10)];
  
  T tree1(10, treeData1);
  T tree2(10, treeData2);
  T tree3(10, treeData3);
  
  FixedCluster<3> cluster;
  cluster.Push(0, tree1, 1);
  cluster.Push(0x400, tree2, 1);
  cluster.Push(0x800, tree3, 1);
  
  cluster.Reserve(0x200, 0x810);
  assert(cluster[0].GetFreeSize() == 0x200);
  assert(cluster[1].GetFreeSize() == 0);
  assert(cluster[2].GetFreeSize() == 0x1f0);
  
  delete[] treeData1;
  delete[] treeData2;
  delete[] treeData3;
}

template <class T>
void TestOperations(const char * treeName) {
  ScopedPass pass("Cluster::[Alloc/Align/Free]() [", treeName, "]");
  
  uint8_t * treeData1 = new uint8_t[T::MemorySize(10)];
  uint8_t * treeData2 = new uint8_t[T::MemorySize(10)];
  uint8_t * treeData3 = new uint8_t[T::MemorySize(10)];
  
  T tree1(10, treeData1);
  T tree2(10, treeData2);
  T tree3(10, treeData3);
  
  FixedCluster<3> cluster;
  cluster.Push(0, tree1, 1);
  cluster.Push(0x400, tree2, 1);
  cluster.Push(0x800, tree3, 1);
  
  UInt addr1;
  bool res = cluster.Alloc(0x400, addr1);
  assert(res);
  assert(addr1 == 0);
  assert(tree1.GetType(Path::Root()) == NodeTypeData);
  cluster.Free(addr1);
  res = cluster.Align(0x400, 0x800, addr1);
  assert(res);
  assert(addr1 == 0);
  assert(tree1.GetType(Path::Root()) == NodeTypeData);
  
  UInt addr2;
  res = cluster.Align(0x400, 0x800, addr2);
  assert(res);
  assert(addr2 == 0x800);
  assert(tree3.GetType(Path::Root()) == NodeTypeData);
  
  assert(tree2.GetType(Path::Root()) == NodeTypeFree);
  UInt addr3;
  res = cluster.Align(0x200, 0x400, addr3);
  assert(res);
  assert(addr3 == 0x400);
  assert(tree2.GetType(Path::Root()) == NodeTypeContainer);
  assert(tree2.GetType(Path(1, 0)) == NodeTypeData);
  assert(tree2.GetType(Path(1, 1)) == NodeTypeFree);
  
  cluster.Free(addr1);
  cluster.Free(addr2);
  cluster.Free(addr3);
  
  assert(tree1.GetType(Path::Root()) == NodeTypeFree);
  assert(tree2.GetType(Path::Root()) == NodeTypeFree);
  assert(tree3.GetType(Path::Root()) == NodeTypeFree);
  
  delete[] treeData1;
  delete[] treeData2;
  delete[] treeData3;
}
