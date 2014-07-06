#ifndef __TEST_MICROTIME_HPP__
#define __TEST_MICROTIME_HPP__

#include <cstdint>
#include <ctime>
#include <sys/time.h>

inline uint64_t Microtime() {
  timeval time;
  gettimeofday(&time, NULL);
  return (uint64_t)time.tv_sec * 1000000L + (uint64_t)time.tv_usec;
}

#endif
