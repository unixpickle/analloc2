#ifndef __SCOPED_BUFFER_HPP__
#define __SCOPED_BUFFER_HPP__

#include <cstdlib>
#include <cstdint>
#include <iostream>

struct ScopedBuffer {
  ScopedBuffer(size_t _size, size_t align = 1) : size(_size) {
    if (align == 1) {
      buffer = malloc(size);
      if (!buffer) {
        std::cerr << "ScopedBuffer(" << size << ", " << align <<
          ") failed." << std::endl;
        abort();
      }
    } else if (posix_memalign(&buffer, align, size)) {
      std::cerr << "ScopedBuffer(" << size << ", " << align <<
        ") failed." << std::endl;
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