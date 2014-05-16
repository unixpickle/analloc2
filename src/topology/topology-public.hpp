namespace ANAlloc {

#define ALLOCLIST_TEMP template <int mc, class T>
#define ALLOCLIST_T AllocatorList<mc, T>

ALLOCLIST_TEMP
ALLOCLIST_T::AllocatorList(const AlligatorType::InitInfo & info) 
  : super(info) { }

ALLOCLIST_TEMP
ALLOCLIST_T::AllocatorList() : super() { }

ALLOCLIST_TEMP
size_t ALLOCLIST_T::BitmapByteCount() {
  size_t res = 0;
  for (int i = 0; i < descriptionCount; i++) {
    res += TreeType::MemorySize(descriptions[i].depth);
  }
  return res;
}

ALLOCLIST_TEMP
void ALLOCLIST_T::GenerateAllocators(uint8_t * buffStart) {
  availableSpace = 0;
  for (int i = 0; i < descriptionCount; i++) {
    TreeType tree(descriptions[i].depth, buffStart);
    buffStart += TreeType::MemorySize(descriptions[i].depth);
    trees[i] = tree;
  
    Allocator<TreeType> al(&trees[i]);
    allocators[i] = al;
  
    availableSpace += pageSize << (descriptions[i].depth - 1);
  }
}

ALLOCLIST_TEMP
void ALLOCLIST_T::Reserve(uintptr_t end) {
  size_t usedSize = 0;
  
  for (int i = 0; i < descriptionCount; i++) {
    Description & desc = descriptions[i];
    if (desc.GetStart() >= end) continue;
    
    size_t reserveSize = desc.GetSize();
    if (desc.Contains(end)) {
      reserveSize = (size_t)(end - desc.GetStart());
    }
    
    uintptr_t numChunks = reserveSize /= info.pageSize;
    if (numChunks * pageSize != reserveSize) {
      numChunks++;
    }
    
    Path p;
    bool res = allocators[i].Alloc(0, p);
    assert(res);
    
    allocators[i].Reserve(p, 0, numChunks);
    usedSize += numChunks * pageSize;
  }
  
  assert(usedSize <= availableSpace);
  availableSpace -= usedSize;
}

#undef ALLOCLIST_T
#undef ALLOCLIST_TEMP

}

