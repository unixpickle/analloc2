#include "log.hpp"

namespace ANAlloc {

uint8_t Log2Ceil(uint64_t value) {
  uint8_t floored = Log2Floor(value);
  if ((1UL << floored) != value) {
    return floored + 1;
  } else {
    return floored;
  }
}

}
