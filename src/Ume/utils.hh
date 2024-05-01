/*
  Copyright (c) 2023, Triad National Security, LLC. All rights reserved.

  This is open source software; you can redistribute it and/or modify it under
  the terms of the BSD-3 License. If software is modified to produce derivative
  works, such modified software should be clearly marked, so as not to confuse
  it with the version available from LANL. Full text of the BSD-3 License can be
  found in the LICENSE.md file, and the full assertion of copyright in the
  NOTICE.md file.
*/

/*!
\file Ume/utils.hh
*/

#include "shm_allocator.hh"

#include <istream>
#include <limits>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#ifndef UME_UTILS_HH
#define UME_UTILS_HH 1

namespace Ume {
inline void skip_line(std::istream &S) {
  S.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

//! Scalar binary write
template <class T> inline void write_bin(std::ostream &os, T const &data) {
  os.write(reinterpret_cast<char const *>(&data),
      static_cast<std::streamsize>(sizeof(T)));
}

//! Scalar binary read
template <class T> inline void read_bin(std::istream &is, T &data) {
  is.read(
      reinterpret_cast<char *>(&data), static_cast<std::streamsize>(sizeof(T)));
}

//! String binary write
template <>
inline void write_bin<std::string>(std::ostream &os, std::string const &data) {
  write_bin(os, data.size());
  os.write(data.c_str(), static_cast<std::streamsize>(data.size()));
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

//! Binary write for UmeVector
template <class T> void write_bin(std::ostream &os, UmeVector<T> const &data) {
  write_bin(os, data.size());
  if (!data.empty()) {
    os.write(reinterpret_cast<const char *>(data.data()),
        static_cast<std::streamsize>(sizeof(T) * data.size()));
  }
  os << '\n';
}

//! Binary read for UmeVector
template <class T> void read_bin(std::istream &is, UmeVector<T> &data) {
  size_t len;
  read_bin(is, len);
  if (len == 0) {
    UmeVector<T> foo;
    data.swap(foo);
  } else {
    data.resize(len);
    is.read(reinterpret_cast<char *>(data.data()),
        static_cast<std::streamsize>(sizeof(T) * len));
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

int init_depth(int const delta);
void debug_attach_point(int const mype);

} // namespace Ume
#endif
