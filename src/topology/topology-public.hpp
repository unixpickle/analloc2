namespace ANAlloc {

template <int mc, class T>
AllocatorList<mc, T>::AllocatorList(size_t _alignment, size_t _minAlignment,
                                    size_t _pageSize, Region * _regions,
                                    int _regionCount) {
  assert((1L << Log2Floor(_alignment)) == _alignment);
  assert((1L << Log2Floor(_pageSize)) == _pageSize);
  alignment = _alignment;
  minAlignment = _minAlignment;
  pageSize = _pageSize;
  regions = _regions;
  regionCount = _regionCount;
  descriptionCount = 0;
  availableSpace = 0;
}

template <int mc, class T>
AllocatorList<mc, T>::AllocatorList() {
  alignment = 0;
  minAlignment = 0;
  pageSize = 0;
  regions = NULL;
  regionCount = 0;
  descriptionCount = 0;
  availableSpace = 0;
}

template <int mc, class T>
void AllocatorList<mc, T>::SetInformation(size_t _alignment,
                                          size_t _minAlignment,
                                          size_t _pageSize,
                                          Region * _regions,
                                          int _regionCount) {
  assert((1L << Log2Floor(_alignment)) == _alignment);
  assert((1L << Log2Floor(_pageSize)) == _pageSize);
  alignment = _alignment;
  minAlignment = _minAlignment;
  pageSize = _pageSize;
  regions = _regions;
  regionCount = _regionCount;
  descriptionCount = 0;
  availableSpace = 0;
}

template <int mc, class T>
void AllocatorList<mc, T>::GenerateDescriptions(bool sorted) {
  while (descriptionCount < mc) {
    Description desc(pageSize);
    if (!FindLargestDescription(desc)) {
      alignment >>= 1;
      if (alignment < minAlignment) break;
      continue;
    }
    InsertDescription(desc, sorted);
  }
}

template <int mc, class T>
size_t AllocatorList<mc, T>::BitmapByteCount() {
  size_t res = 0;
  for (int i = 0; i < descriptionCount; i++) {
    res += T::MemorySize(descriptions[i].depth);
  }
  return res;
}

template <int mc, class T>
void AllocatorList<mc, T>::GenerateAllocators(uint8_t * buffStart) {
  for (int i = 0; i < descriptionCount; i++) {
    T tree(descriptions[i].depth, buffStart);
    buffStart += T::MemorySize(descriptions[i].depth);
    trees[i] = tree;
    
    Allocator<T> al(&trees[i]);
    allocators[i] = al;
    
    availableSpace += pageSize << (descriptions[i].depth - 1);
  }
}

template <int mc, class T>
void AllocatorList<mc, T>::Reserve(const Region & reg) {
  size_t usedSize = 0;
  
  for (int i = 0; i < descriptionCount; i++) {
    Description & desc = descriptions[i];
    uintptr_t descEnd = desc.start + desc.GetSize();
    if (reg.GetStart() > descEnd) {
      continue;
    } else if (reg.GetEnd() <= desc.start) {
      continue;
    }
    
    uintptr_t chunkStart, chunkEnd;
    if (reg.GetStart() > desc.start) {
      chunkStart = reg.GetStart();
    } else {
      chunkStart = desc.start;
    }
    if (reg.GetEnd() < descEnd) {
      chunkEnd = reg.GetEnd();
    } else {
      chunkEnd = descEnd;
    }
    
    Path startPath = FloorBasePath(desc, chunkStart);
    size_t pathCount = CountBaseNodes(desc, chunkStart, chunkEnd);
    
    if (!pathCount) continue;
    
    Path p;
    bool result = allocators[i].Alloc(0, p);
    assert(result);
    
    size_t dataSize = 1L << (desc.depth - 1);
    allocators[i].Reserve(p, startPath, (uintptr_t)pathCount);
    usedSize += pageSize * pathCount;
  }
  
  assert(usedSize <= availableSpace);
  availableSpace -= usedSize;
}

template <int mc, class T>
const Description * AllocatorList<mc, T>::GetDescriptions() {
  return static_cast<Description * const>(descriptions);
}

template <int mc, class T>
int AllocatorList<mc, T>::GetDescriptionCount() {
  return descriptionCount;
}

template <int mc, class T>
T * AllocatorList<mc, T>::GetTrees() {
  return trees;
}

template <int mc, class T>
Allocator<T> * AllocatorList<mc, T>::GetAllocators() {
  return allocators;
}

template <int mc, class T>
uintptr_t AllocatorList<mc, T>::PointerForPath(int allocIndex, Path p) {
  Description & desc = descriptions[allocIndex];
  int depth = PathDepth(p);
  
  size_t eachSize = (pageSize << (desc.depth - depth - 1));
  return desc.start + eachSize * PathIndex(p);
}

template <int mc, class T>
bool AllocatorList<mc, T>::PathForPointer(uintptr_t ptr, Path & path,
                                          int & i) {
  for (i = 0; i < descriptionCount; i++) {
    Description & desc = descriptions[i];
    uintptr_t descEnd = desc.start + desc.GetSize();
    if (ptr < desc.start) continue;
    if (ptr >= descEnd) continue;
    
    // we found our descriptor
    uintptr_t baseIndex = (ptr - desc.start) / pageSize;
    if (!allocators[i].Find(baseIndex, path)) {
      return false;
    }
    return true;
  }
  return false;
}

template <int mc, class T>
bool AllocatorList<mc, T>::AllocPath(size_t size, unsigned int alignLog,
                                     Path & p, int & i) {
  size_t alignSize = (1L << alignLog);
  size_t grabSize = size > alignSize ? size : alignSize;
  int grabPower = Log2Ceil(grabSize) - Log2Ceil(pageSize);
  if (grabPower < 0) grabPower = 0;

  for (i = 0; i < descriptionCount; i++) {
    Description & desc = descriptions[i];
    uintptr_t start = desc.start;
    // make sure `start` is aligned to `align`
    if (start & (alignSize - 1)) continue;
    // make sure its big enough
    if (desc.depth <= grabPower) continue;
    // calculate the depth of the node we need
    int reqDepth = desc.depth - grabPower - 1;
    if (allocators[i].Alloc(reqDepth, p)) {
      return true;
    }
  }
  if (alignLog > 0) return BadAlloc(size, alignLog, p, i);
  return false;
}

template <int mc, class T>
bool AllocatorList<mc, T>::BadAlloc(size_t size, size_t alignLog,
                                    Path & p, int & i) {
  size_t alignSize = (1L << alignLog);
  size_t grabSize = size > alignSize ? size : alignSize;
  
  return AllocPath(grabSize * 2, 0, p, i);
}

template <int mc, class T>
bool AllocatorList<mc, T>::AllocPointer(size_t size,
                                        size_t align,
                                        uintptr_t & out,
                                        size_t * sizeOut) {
  int alignLog = Log2Floor(align);
  assert((1L << alignLog) == align);

  Path p;
  int i;
  if (!AllocPath(size, alignLog, p, i)) {
    return false;
  }
  
  out = PointerForPath(i, p);
  
  size_t usedSize = 0;
  int power = descriptions[i].depth - PathDepth(p) - 1;
  usedSize = pageSize << power;
  if (sizeOut) *sizeOut = usedSize;
  
  // make sure the actual returned address is aligned (it might not be if)
  // our call got forwarded to BadAlloc().
  if (out % align) {
    if (sizeOut) *sizeOut -= align - (out % align);
    out += align - (out % align);
  }
  
  availableSpace -= usedSize;
  
  return true;
}

template <int mc, class T>
void AllocatorList<mc, T>::FreePointer(uintptr_t ptr) {
  Path p;
  int i;
  if (!PathForPointer(ptr, p, i)) return;
  
  int depth = descriptions[i].depth;
  size_t dataSize = pageSize << (depth - 1 - PathDepth(p));
  availableSpace += dataSize;
  
  allocators[i].Free(p);
}

template <int mc, class T>
size_t AllocatorList<mc, T>::AvailableSpace() {
  return availableSpace;
}

}

