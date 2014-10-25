#ifndef __SCOPED_BUFFER_HPP__
#define __SCOPED_BUFFER_HPP__

#include <cstdlib>
#include <cstdint>
#include <iostream>

template <typename T>
struct ScopedBuffer {
  ScopedBuffer(size_t _size, size_t align = 1) : size(_size) {
    if (int res = posix_memalign(&buffer, align, size)) {
      std::cerr << "ScopedBuffer(" << size << ", " << align <<
        ") failed." << res << std::endl;
      abort();
    }
  }
  
  ~ScopedBuffer() {
    free(buffer);
  }
  
  operator uintptr_t() {
    return (uintptr_t)buffer;
  }
  
  void * buffer;
  size_t size;
};

#endif