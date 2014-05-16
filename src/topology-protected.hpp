namespace ANAlloc {

template <int mc, class T>
bool AllocatorList<mc, T>::FindLargestDescription(Description & desc) {
  desc.depth = 0;
  for (int i = 0; i < regionCount; i++) {
    Region & reg = regions[i];
    Description result;
    if (!RegionLargestFree(reg, result)) continue;
    
    if (result.depth > desc.depth) {
      desc.depth = result.depth;
      desc.start = result.start;
    } 
  }
  return desc.depth != 0;
}

template <int mc, class T>
bool AllocatorList<mc, T>::RegionLargestFree(Region & reg,
                                             Description & desc) {
  uintptr_t currentLoc = NextFreeAligned(reg, reg.GetStart());    
  desc.depth = 0;
  
  while (currentLoc < reg.GetEnd()) {
    uintptr_t nextFree;
    uintptr_t endBreak = NextBreak(reg, currentLoc, &nextFree);
    
    size_t size = (size_t)(endBreak - currentLoc);
    int depth = SizeDepth(size);
    if (depth > desc.depth) {
      desc.depth = depth;
      desc.start = currentLoc;
    }
    
    currentLoc = NextFreeAligned(reg, nextFree);
  }
  return desc.depth != 0;
}

template <int mc, class T>
uintptr_t AllocatorList<mc, T>::NextFreeAligned(Region & reg, uintptr_t loc) {
  while (loc < reg.GetEnd()) {
    if (loc % alignment) {
      loc += alignment - (loc % alignment);
    }
    if (loc >= reg.GetEnd()) break;
    
    // check if there is a region which contains this location
    bool isContained = false;
    for (int i = 0; i < descriptionCount; i++) {
      Description & desc = descriptions[i];
      if (desc.start > loc) continue;
      if (desc.start + DepthSize(desc.depth) > loc) {
        isContained = true;
        loc = desc.start + DepthSize(desc.depth);
        break;
      }
    }
    if (!isContained) {
      return loc;
    }
  }
  return reg.GetEnd();
}

template <int mc, class T>
uintptr_t AllocatorList<mc, T>::NextBreak(Region & reg, uintptr_t loc,
                                          uintptr_t * nextFree) {
  uintptr_t nextBreak = reg.GetStart() + reg.GetSize();
  if (nextFree) *nextFree = nextBreak;
  for (int i = 0; i < descriptionCount; i++) {
    Description & desc = descriptions[i];
    if (desc.start < loc) continue;
    if (desc.start >= nextBreak) continue;
    nextBreak = descriptions[i].start;
    if (nextFree) {
      *nextFree = nextBreak + DepthSize(desc.depth);
    }
  }
  return nextBreak;
}

template <int mc, class T>
size_t AllocatorList<mc, T>::DepthSize(int depth) {
  if (!depth) return 0;
  return (pageSize << (depth - 1));
}

template <int mc, class T>
int AllocatorList<mc, T>::SizeDepth(size_t size) {
  if (!size) return 0;
  
  for (int i = 1; i < 64; i++) {
    size_t aSize = pageSize << (i - 1);
    if (aSize > size) {
      return i - 1;
    } else if (aSize == size) {
      return i;
    }
  }
  
  return 0;
}

template <int mc, class T>
Path AllocatorList<mc, T>::FloorBasePath(Description & desc,
                                         uintptr_t byteIndex) {
  uintptr_t diff = byteIndex - desc.start;
  return (Path)(diff / pageSize);
}

template <int mc, class T>
size_t AllocatorList<mc, T>::CountBaseNodes(Description & desc,
                                            uintptr_t start,
                                            uintptr_t end) {
  Path pathStart = FloorBasePath(desc, start);
  uintptr_t realStart = desc.start + (pathStart * pageSize);
  uintptr_t diff = end - realStart;
  return (size_t)(diff / pageSize) + (diff % pageSize ? 1L : 0L);
}

template <int mc, class T>
void AllocatorList<mc, T>::InsertDescription(const Description & desc,
                                             bool sorted) {
  assert(descriptionCount < mc);

  if (!sorted) {
    descriptions[descriptionCount++] = desc;
    return;
  }

  int insIndex = descriptionCount;
  for (int i = 0; i < descriptionCount; i++) {
    if (descriptions[i].start > desc.start) {
      insIndex = i;
      break;
    }
  }
  for (int i = descriptionCount; i > insIndex; i--) {
    descriptions[i] = descriptions[i - 1];
  }
  descriptions[insIndex] = desc;
  descriptionCount++;
}

}

