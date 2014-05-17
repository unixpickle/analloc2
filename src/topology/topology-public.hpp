namespace ANAlloc {

#define ALLOCLIST_TEMP template <int mc, class TreeType>
#define ALLOCLIST_T AllocatorList<mc, TreeType>

ALLOCLIST_TEMP
ALLOCLIST_T::AllocatorList(const typename AlligatorType::InitInfo & info) 
  : super(info) { }

ALLOCLIST_TEMP
ALLOCLIST_T::AllocatorList(size_t alignment, size_t minAlignment,
                           size_t pageSize, Region * regions,
                           int regionCount) {
  this->SetInfo(typename AlligatorType::InitInfo(alignment, minAlignment,
                                                 pageSize, regions,
                                                 regionCount));
}

ALLOCLIST_TEMP
ALLOCLIST_T::AllocatorList() : super() { }

ALLOCLIST_TEMP
size_t ALLOCLIST_T::BitmapByteCount() {
  size_t res = 0;
  for (int i = 0; i < GetDescriptionCount(); i++) {
    res += TreeType::MemorySize(GetDescriptions()[i].GetDepth());
  }
  return res;
}

ALLOCLIST_TEMP
void ALLOCLIST_T::GenerateAllocators(uint8_t * buffStart) {
  availableSpace = 0;
  for (int i = 0; i < GetDescriptionCount(); i++) {
    TreeType tree(GetDescriptions()[i].GetDepth(), buffStart);
    buffStart += TreeType::MemorySize(GetDescriptions()[i].GetDepth());
    trees[i] = tree;
  
    Allocator<TreeType> al(&trees[i]);
    allocators[i] = al;
  
    availableSpace += GetPageSize() << (GetDescriptions()[i].GetDepth() - 1);
  }
}

ALLOCLIST_TEMP
void ALLOCLIST_T::Reserve(uintptr_t end) {
  size_t usedSize = 0;
  
  for (int i = 0; i < GetDescriptionCount(); i++) {
    Description & desc = GetDescriptions()[i];
    if (desc.GetStart() >= end) continue;
    
    size_t reserveSize = desc.GetSize();
    if (desc.Contains(end)) {
      reserveSize = (size_t)(end - desc.GetStart());
    }
    
    uintptr_t numChunks = reserveSize / GetPageSize();
    if (numChunks * GetPageSize() != reserveSize) {
      numChunks++;
    }
    
    Path p;
    bool res = allocators[i].Alloc(0, p);
    assert(res);
    
    allocators[i].Reserve(p, 0, numChunks);
    usedSize += numChunks * GetPageSize();
  }
  
  assert(usedSize <= availableSpace);
  availableSpace -= usedSize;
}

ALLOCLIST_TEMP
size_t ALLOCLIST_T::GetPageSize() {
  return this->info.pageSize;
}

ALLOCLIST_TEMP
Description * ALLOCLIST_T::GetDescriptions() {
  return this->descriptions;
}

ALLOCLIST_TEMP
int ALLOCLIST_T::GetDescriptionCount() {
  return this->descriptionCount;
}

ALLOCLIST_TEMP
TreeType * ALLOCLIST_T::GetTrees() {
  return trees;
}

ALLOCLIST_TEMP
Allocator<TreeType> * ALLOCLIST_T::GetAllocators() {
  return allocators;
}

ALLOCLIST_TEMP
size_t ALLOCLIST_T::GetAvailableSpace() {
  return availableSpace;
}

ALLOCLIST_TEMP
uintptr_t ALLOCLIST_T::PointerForPath(int allocIndex, Path p) {
  Description & desc = GetDescriptions()[allocIndex];
  int depth = PathDepth(p);

  size_t eachSize = (GetPageSize() << (desc.GetDepth() - depth - 1));
  return desc.GetStart() + eachSize * PathIndex(p);
}

ALLOCLIST_TEMP
bool ALLOCLIST_T::PathForPointer(uintptr_t ptr, Path & path, int & i) {
  for (i = 0; i < GetDescriptionCount(); i++) {
    Description & desc = GetDescriptions()[i];
    if (!desc.Contains(ptr)) continue;
  
    // we found our descriptor
    uintptr_t baseIndex = (ptr - desc.GetStart()) / GetPageSize();
    return allocators[i].Find(baseIndex, path);
  }
  return false;
}

ALLOCLIST_TEMP
void ALLOCLIST_T::FreePointer(uintptr_t ptr) {
  Path p;
  int i;
  if (!PathForPointer(ptr, p, i)) return;

  int depth = GetDescriptions()[i].GetDepth();
  size_t dataSize = GetPageSize() << (depth - 1 - PathDepth(p));
  availableSpace += dataSize;

  allocators[i].Free(p);
}

ALLOCLIST_TEMP
bool ALLOCLIST_T::AllocPointer(size_t size, size_t align,
                               uintptr_t & out, size_t * sizeOut) {
  return AllocGeneral(size, align, out, sizeOut, 1, UINTPTR_MAX);
}

ALLOCLIST_TEMP
bool ALLOCLIST_T::AllocAscending(size_t size, size_t align,
                                 uintptr_t & out, size_t * sizeOut) {
  return AllocGeneral(size, align, out, sizeOut, 1, UINTPTR_MAX);
}

ALLOCLIST_TEMP
bool ALLOCLIST_T::AllocAscending(size_t size, size_t align,
                                 uintptr_t & out, size_t * sizeOut,
                                 uintptr_t maximum) {
  return AllocGeneral(size, align, out, sizeOut, 1, maximum);
}

ALLOCLIST_TEMP
bool ALLOCLIST_T::AllocDescending(size_t size, size_t align,
                                  uintptr_t & out, size_t * sizeOut) {
  return AllocGeneral(size, align, out, sizeOut, -1, 0);
}

ALLOCLIST_TEMP
bool ALLOCLIST_T::AllocDescending(size_t size, size_t align,
                                  uintptr_t & out, size_t * sizeOut,
                                  uintptr_t minimum) {
  return AllocGeneral(size, align, out, sizeOut, -1, minimum);
}

#undef ALLOCLIST_T
#undef ALLOCLIST_TEMP

}
