/*!
\file Ume/Comm_Buffers.hh
*/

#ifndef COMM_BUFFERS_HH
#define COMM_BUFFERS_HH 1

#include "Ume/Comm_Neighbors.hh"
#include "Ume/DS_Types.hh"
#include <iterator>
#include <type_traits>

namespace Ume {
namespace Comm {

enum class Op { OVERWRITE, MAX, MIN, SUM };

template <class DST, class OutputIt>
OutputIt pack(DST const &val, OutputIt d_first) {
  if constexpr (std::is_scalar_v<DST>) {
    *d_first++ = val;
  } else {
    std::copy(std::begin(val), std::end(val), d_first);
  }
  return d_first;
}

template <class InputIt, class DST> InputIt unpack(InputIt first, DST &val) {
  if constexpr (std::is_scalar_v<DST>) {
    val = *first++;
  } else {
    for (auto &e : val) {
      e = *first++;
    }
  }
  return first;
}

/* The idea is that we have one aggregated communication buffer, rather than one
   per PE, so that we can fill the buffer on a GPU using a simple map. */
template <class T> class Buffers {
public:
  using base_type = typename DS_Type_Info<T>::base_type;

public:
  Buffers() = delete;
  explicit Buffers(Neighbors const &neighs);

  /* You can write your own buffer pack/unpack functions using these
     functions */
  base_type *get_buf() { return buf.data(); }
  constexpr size_t num_entries() const { return b2e_m.size(); }
  size_t const *buf2ent() const { return b2e_m.data(); }

  /* Convenience implementation.  If you are doing a GPU implementation, it is
     probably better to move this operation to the GPU, so that you don't have
     to move the entire field to the host and back. */
  void pack(T const &field);
  void unpack(T &field, Op op);

public:
  struct Remote {
    int pe;
    size_t buf_offset;
    int buf_len;
  };

  // Per-remote information;
  std::vector<Remote> remotes;
  std::vector<size_t> b2e_m;
  std::vector<base_type> buf;
};

} // namespace Comm
} // namespace Ume

#endif
