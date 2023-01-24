/*!
\file Ume/Comm_Transport.hh
*/

#ifndef COMM_TRANSPORT_HH
#define COMM_TRANSPORT_HH 1

#include "Ume/Comm_Neighbors.hh"
#include "Ume/DS_Types.hh"
#include <iterator>
#include <type_traits>

namespace Ume {
namespace Comm {

enum class Op { SUM };

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

class Transport;

/* The idea is that we have one aggregated communication buffer, rather than one
   per PE, so that we can fill the buffer on a GPU using a simple map. */
template <class T> class Buffers {
public:
  using base_type = typename DS_Type_Info<T>::base_type;

public:
  explicit Buffers(Neighbors const &neighs) : remotes_{neighs.size()} {
    auto const entries_per_elem = DS_Type_Info<T>::elem_len;
    size_t count{0};
    for (size_t ni = 0; ni < neighs.size(); ++ni) {
      // Set the remote PE
      remotes_[ni].pe = neighs[ni].pe;
      // Set the offset of this PE's data into the aggregated buffer
      remotes_[ni].buf_offset = count * entries_per_elem;
      // Set the length of this PE's data in the aggregated buffer
      remotes_[ni].buf_len = neights[ni].elements.size() * entries_per_elem;
      // Add these elements to the bufIdx_to_EntIdx map
      b2e_m_.insert(
          b2e_m_.end(), neighs[ni].elements.begin(), neighs[ni].elements.end());
      // Advance the total count
      count += neighs[ni].elements.size();
    }
    // Resize the aggregated buffer
    buf_.resize(count * entries_per_elem);
  }

  /* You can write your own buffer pack/unpack functions using these
     functions */
  base_type *get_buf() { return buf_.data(); }
  constexpr size_t num_entries() const { return b2e_m_.size(); }
  size_t const *buf2ent() const { return b2e_m_.data(); }

  /* Convenience implementation.  If you are doing a GPU implementation, it is
     probably better to move this operation to the GPU, so that you don't have
     to move the entire field to the host and back. */
  void pack(T const &field) {
    const size_t N = num_entries();
    base_type *buf = get_buf();
    size_t const *map = buf2ent();
    for (size_t i = 0; i < N; ++i) {
      buf = Comm::pack(field[map[i]], buf);
    }
  }

  void unpack(T &field) {
    const size_t N = num_entries();
    base_type const *buf = get_buf();
    size_t const *map = buf2ent();
    for (size_t i = 0; i < N; ++i) {
      buf = Comm::unpack(buf, field[map[i]]);
    }
  }

private:
  friend class Transport;
  struct Remote {
    int pe;
    size_t buf_offset;
    size_t buf_len;
  };

  // Per-remote information;
  std::vector<Remote> remotes_;
  std::vector<size_t> b2e_m_;
  std::vector<base_type> buf_;
};

class Transport {
public:
  virtual void exchange(Buffers<DS_Types::INTV_T> const & /*sends*/,
      Buffers<DS_Types::INTV_T> & /*recvs*/) {}
  virtual void exchange(Buffers<DS_Types::DBLV_T> const & /*sends*/,
      Buffers<DS_Types::INTV_T> & /*recvs*/) {}
  virtual void exchange(Buffers<DS_Types::VEC3V_T> const & /*sends*/,
      Buffers<DS_Types::VEC3V_T> & /*recvs*/) {}
  virtual int stop() { return -1; };
  virtual ~Transport() = default;
};

} // namespace Comm
} // namespace Ume

#endif
