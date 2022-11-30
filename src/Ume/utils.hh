/*!
\file utils.hh
*/

#include <istream>
#include <limits>
#include <ostream>
#include <vector>

#ifndef UTILS_HH
#define UTILS_HH 1

namespace Ume {
inline void skip_line(std::istream &S) {
  S.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

//! Scalar binary write
template <class T> inline void write_bin(std::ostream &os, T const &data) {
  os.write(reinterpret_cast<char const *>(&data), sizeof(T));
}

//! Scalar binary read
template <class T> inline void read_bin(std::istream &is, T &data) {
  is.read(reinterpret_cast<char *>(&data), sizeof(T));
}

//! Binary write for std::vector
template <class T>
void write_bin(std::ostream &os, std::vector<T> const &data) {
  write_bin(os, data.size());
  if (!data.empty()) {
    os.write(reinterpret_cast<const char *>(&data[0]), sizeof(T) * data.size());
  }
  os << '\n';
}

//! Binary read for std::vector
template <class T> void read_bin(std::istream &is, std::vector<T> &data) {
  size_t len;
  read_bin(is, len);
  if (len == 0) {
    data.clear();
  } else {
    data.resize(len);
    is.read(reinterpret_cast<char *>(&data[0]), sizeof(T) * len);
  }
  Ume::skip_line(is);
}

} // namespace Ume
#endif
