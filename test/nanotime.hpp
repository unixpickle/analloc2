#ifndef __TEST_NANOTIME_HPP__
#define __TEST_NANOTIME_HPP__

#include <cstdint>
#include <ctime>
#include <sys/time.h>

inline uint64_t Nanotime() {
  timeval time;
  gettimeofday(&time, NULL);
  return (uint64_t)time.tv_sec * 1000000000L + (uint64_t)time.tv_usec * 1000L;
}

#endif
