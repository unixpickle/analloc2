namespace ANAlloc {

#define ALLIGATOR_TEMP template <int N>
#define ALLIGATOR_T Alligator<N>

ALLIGATOR_TEMP
bool ALLIGATOR_T::FindLargestFree(Description & output) {
  output.SetDepth(0);
  for (int i = 0; i < info.regionCount; i++) {
    Region & reg = info.regions[i];
    Description result(info.pageSize);
    if (!FindLargestFree(reg, result)) continue;
  
    if (result.GetDepth() > output.GetDepth()) {
      output = result;
    } 
  }
  return output.GetDepth() != 0;
}

ALLIGATOR_TEMP
bool ALLIGATOR_T::FindLargestFree(Region & region, Description & output) {
  output.SetDepth(0);
  
  uintptr_t location = FindNextFreeAligned(region, region.GetStart());
  while (location < region.GetEnd()) {
    int nextDesc = FindNextDescription(region, location);
    size_t freeSpace = 0;
    if (nextDesc < 0) {
      freeSpace = (size_t)(region.GetEnd() - location);
    } else {
      freeSpace = (size_t)(descriptions[nextDesc].GetStart() - location);
    }
    
    if (freeSpace >= info.pageSize) {
      int descDepth = Log2Floor(freeSpace) - Log2Floor(info.pageSize) + 1;
      if (descDepth > output.GetDepth()) {
        output.SetDepth(descDepth);
        output.SetStart(location);
      }
    }
    
    location = FindNextFreeAligned(region, location + freeSpace);
  }
  
  return output.GetDepth() != 0;
}

ALLIGATOR_TEMP
uintptr_t ALLIGATOR_T::FindNextFreeAligned(Region & region, uintptr_t loc) {
  assert(loc >= region.GetStart());
  assert(loc <= region.GetEnd());
  while (loc < region.GetEnd()) {
    if (loc % info.alignment) {
      loc += info.alignment - (loc % info.alignment);
    }
    if (loc >= region.GetEnd()) break;
    
    // see if the location is contained by any region
    bool isContained = false;
    for (int i = 0; i < descriptionCount; i++) {
      if (descriptions[i].Contains(loc)) {
        isContained = true;
        loc = descriptions[i].GetEnd();
        break;
      }
    }
    if (!isContained) return loc;
  }
  return region.GetEnd();
}

ALLIGATOR_TEMP
int ALLIGATOR_T::FindNextDescription(Region & region, uintptr_t loc) {
  uintptr_t curStart = ~(uintptr_t)0;
  int curIdx = -1;
  
  for (int i = 0; i < descriptionCount; i++) {
    if (!region.Contains(descriptions[i].GetStart())) continue;
    if (descriptions[i].GetStart() < loc) continue;
    if (descriptions[i].GetStart() > curStart) continue;
    curIdx = i;
    curStart = descriptions[i].GetStart();
  }
  
  return curIdx;
}

ALLIGATOR_TEMP
void ALLIGATOR_T::InsertDescription(const Description & desc, bool sorted) {
  assert(descriptionCount < MaxDescriptionCount);

  if (!sorted) {
    descriptions[descriptionCount++] = desc;
    return;
  }

  int insIndex = descriptionCount;
  for (int i = 0; i < descriptionCount; i++) {
    if (descriptions[i].GetStart() > desc.GetStart()) {
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

#undef ALLIGATOR_T
#undef ALLIGATOR_TEMP

}