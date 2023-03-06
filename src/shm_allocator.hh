extern "C" {
#include "shm_malloc.h"
}

#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>

template <class T> struct shm_allocator {
  typedef T value_type;

  shm_allocator() = default;

  template <class U>
  constexpr shm_allocator(const shm_allocator<U> &) noexcept {}

  [[nodiscard]] T *allocate(std::size_t n) {
    if (n > std::numeric_limits<std::size_t>::max() / sizeof(T))
      throw std::bad_array_new_length();

    if (auto p = static_cast<T *>(shm_malloc(n * sizeof(T)))) {
      report(p, n);
      return p;
    }

    throw std::bad_alloc();
  }

  void deallocate(T *p, std::size_t n) noexcept {
    report(p, n, 0);
    shm_free(p);
  }

private:
  void report(T *p, std::size_t n, bool alloc = true) const {
    std::cout << (alloc ? "Alloc: " : "Dealloc: ") << sizeof(T) * n
              << " bytes at " << std::hex << std::showbase
              << reinterpret_cast<void *>(p) << std::dec << '\n';
  }
};

template <class T, class U>
bool operator==(const shm_allocator<T> &, const shm_allocator<U> &) {
  return true;
}

template <class T, class U>
bool operator!=(const shm_allocator<T> &, const shm_allocator<U> &) {
  return false;
}

template <class T>
#ifdef USE_SCORIA
using UmeVector = std::vector<T, shm_allocator<T>>;
#else
using UmeVector = std::vector<T>;
#endif
