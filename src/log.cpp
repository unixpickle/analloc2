#include "log.hpp"

namespace ANAlloc {

int Log2Ceil(UInt value) {
  int floored = Log2Floor(value);
  if ((1UL << floored) != value) {
    return floored + 1;
  } else {
    return floored;
  }
}

}
