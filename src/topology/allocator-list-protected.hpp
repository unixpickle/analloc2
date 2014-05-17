namespace ANAlloc {

#define ALLOCLIST_TEMP template <int mc, class T>
#define ALLOCLIST_T AllocatorList<mc, T>

ALLOCLIST_TEMP
bool ALLOCLIST_T::AllocPath(size_t size, size_t align, Path & p, int & i,
                            int direction, uintptr_t boundary) {
  size_t grabSize = size > align ? size : align;
  int grabPower = Log2Ceil(grabSize) - Log2Ceil(GetPageSize());
  if (grabPower < 0) grabPower = 0;

  i = (direction > 0 ? 0 : GetDescriptionCount() - 1);
  for (; i >= 0 && i < GetDescriptionCount(); i += direction) {
    Description & desc = GetDescriptions()[i];
    if (direction > 0) {
      if (desc.GetEnd() - 1 > boundary && desc.GetEnd() > 0) {
        break;
      }
    } else {
      if (desc.GetStart() < boundary) {
        break;
      }
    }
    
    uintptr_t start = desc.GetStart();
    // make sure `start` is aligned to `align`
    if (start & (align - 1)) continue;
    // make sure its big enough
    if (desc.GetDepth() <= grabPower) continue;
    // calculate the depth of the node we need
    int reqDepth = desc.GetDepth() - grabPower - 1;
    if (allocators[i].Alloc(reqDepth, p)) {
      return true;
    }
  }
  
  if (align > 1) {
    return AllocPath(grabSize * 2, 1, p, i, direction, boundary);
  }
  
  return false;
}

ALLOCLIST_TEMP
bool ALLOCLIST_T::AllocGeneral(size_t size, size_t align, uintptr_t & out,
                               size_t * sizeOut, int direction,
                               uintptr_t boundary) {
  assert(((size_t)1 << Log2Floor(align)) == align);

  Path p;
  int i;
  if (!AllocPath(size, align, p, i, direction, boundary)) {
    return false;
  }

  out = PointerForPath(i, p);

  size_t usedSize = 0;
  int power = GetDescriptions()[i].GetDepth() - PathDepth(p) - 1;
  usedSize = GetPageSize() << power;
  if (sizeOut) *sizeOut = usedSize;

  // Make sure the actual returned address is aligned--it might not be if the
  // requested alignment could only be attained by allocating twice the needed
  // amount.
  if (out % align) {
    if (sizeOut) *sizeOut -= align - (out % align);
    out += align - (out % align);
  }

  availableSpace -= usedSize;

  return true;
}

#undef ALLOCLIST_T
#undef ALLOCLIST_TEMP

}

