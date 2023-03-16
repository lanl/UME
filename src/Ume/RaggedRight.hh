/*!
  \file Ume/RaggedRight.hh
*/

#ifndef UME_RAGGEDRIGHT_HH
#define UME_RAGGEDRIGHT_HH

#include <span>
#include <vector>

namespace Ume {

//! An array-of-arrays, where each array can be a different length
/*! Implemented as three arrays: the data for primary index `n` is stored in the
    array `data`, in the half-open interval [bidx[n]..eidx[n]).
 */
template <class T> struct RaggedRight {
  RaggedRight() = default;
  explicit RaggedRight(int base_size) { init(base_size); }
  void init(int const len) {
    bidx.assign(len, 0);
    eidx.assign(len, 0);
  }

  bool operator==(RaggedRight<T> const &rhs) const {
    return (bidx == rhs.bidx && eidx == rhs.eidx && data == rhs.data);
  }

  std::span<T> operator[](int const n) {
    return std::span(data.begin() + bidx[n], size(n));
  }
  std::span<T const> const operator[](int const n) const {
    return std::span(data.cbegin() + bidx[n], size(n));
  }

  //! Copy the contents of the range [`b`..`e`) to element `n`
  /*! Note that if `n` has already been assigned data, it will be abandonend in
      the `data` array! */
  template <class IT> void assign(int const n, IT const b, IT const e) {
    bidx.at(n) = static_cast<int>(data.size());
    data.insert(data.end(), b, e);
    eidx.at(n) = static_cast<int>(data.size());
  }

  //! Return the length of the n'th array
  constexpr int size(int const n) const { return eidx[n] - bidx[n]; }

private:
  std::vector<int> bidx, eidx;
  std::vector<T> data;
};

} // namespace Ume

#endif
