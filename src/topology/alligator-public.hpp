namespace ANAlloc {

#define ALLIGATOR_TEMP template <int N>
#define ALLIGATOR_T Alligator<N>

ALLIGATOR_TEMP
ALLIGATOR_T::InitInfo::InitInfo(const InitInfo & info) {
  *this = info;
}

ALLIGATOR_TEMP
ALLIGATOR_T::InitInfo & ALLIGATOR_T::InitInfo::operator=(
  const ALLIGATOR_T::InitInfo & info)
  : alignment(info.alignment), minAlignment(info.minAlignment),
    pageSize(info.pageSize), regions(info.regions),
    regionCount(info.regionCount) { }

ALLIGATOR_TEMP
ALLIGATOR_T::Alligator() : descriptionCount(0) { }

ALLIGATOR_TEMP
ALLIGATOR_T::Alligator(const InitInfo & _info)
  : info(_info), descriptionCount(0) { }

ALLIGATOR_TEMP
void ALLIGATOR_T::SetInfo(const InitInfo & _info) {
  info = _info;
}

ALLIGATOR_TEMP
void ALLIGATOR_T::GenerateDescriptions(bool sorted = false) {
  while (descriptionCount < MaxDescriptionCount) {
    Description desc(info.pageSize);
    if (!FindLargestFree(desc)) {
      info.alignment >>= 1;
      if (info.alignment < info.minAlignment) break;
    } else {
      InsertDescription(desc, sorted);
    }
  }
}

#undef ALLIGATOR_T
#undef ALLIGATOR_TEMP

}
