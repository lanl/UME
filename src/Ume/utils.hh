/*!
\file utils.hh
*/

#include <istream>
#include <limits>
#include <memory>
#include <ostream>
#include <string>
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

//! String binary write
template <>
inline void write_bin<std::string>(std::ostream &os, std::string const &data) {
  write_bin(os, data.size());
  os.write(data.c_str(), data.size());
}

//! String binary read
template <>
inline void read_bin<std::string>(std::istream &is, std::string &data) {
  size_t len;
  read_bin(is, len);
  if (len == 0) {
    data.clear();
  } else {
    std::unique_ptr<char[]> buf(new char[len]);
    is.read(buf.get(), len);
    data = std::string{buf.get(), len};
  }
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

inline std::string ltrim(const std::string &s) {
  const std::string WHITESPACE{" \n\r\t\f\v"};
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

inline std::string rtrim(const std::string &s) {
  const std::string WHITESPACE{" \n\r\t\f\v"};
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

inline std::string trim(const std::string &s) { return rtrim(ltrim(s)); }

} // namespace Ume
#endif
