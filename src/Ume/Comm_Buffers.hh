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

//! Operations performed during gather communications
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

//! A collection of communication buffers for an entity
/*! The idea is that we have one aggregated communication buffer array, rather
   than one per PE, so that we can fill the buffer on a GPU using a simple
   map. */
template <class T> class Buffers {
public:
  using base_type = typename DS_Type_Info<T>::base_type;

public:
  //! Initialize based on entity neighbors
  /*! This creates a single communication buffer and a buffer index to entity
      index map that spans all of the neighboring partitions. */
  explicit Buffers(Neighbors const &neighs);
  Buffers() = delete;

  /* You can write your own buffer pack/unpack functions using these
     functions */
  //! Get the entire buffer array
  base_type *get_buf() { return buf.data(); }
  //! The total number of entities being sent/receieved from all remotes
  constexpr size_t num_entries() const { return b2e_m.size(); }
  //! The map from buffer index to entity index.
  size_t const *buf2ent() const { return b2e_m.data(); }

  //! Fill the buffer from an entity-wide field
  /*! Convenience implementation.  If you are doing a GPU implementation, it is
    probably better to move this operation to the GPU, so that you don't have
     to move the entire field to the host and back. */
  void pack(T const &entity_field);
  //! Scatter from buffer to field, combining field elements with Op
  /*! You may be getting values for a single element from multiple remotes, so
      Op is used to combine those with the existing value. */
  void unpack(T &entity_field, Op op);

public:
  //! Information for one remote process
  struct Remote {
    int pe; //!< the addess of the remote process
    size_t buf_offset; //!< offset of this remote into the b2e_m array
    int buf_len; //!< the number of entities exchanged with this remote
  };

  //! The list of remotes
  std::vector<Remote> remotes;

  //! A map from buffer indices to entity indices
  /*! ei = b2e_m[bi] means that the data in buf[bi] corresponds to the data in
      entity[ei]. The buffer indices come from Remote::buf_offset and
      Remove::buf_len. */
  std::vector<size_t> b2e_m;

  //! The actual communication buffer, used for all Remotes.
  std::vector<base_type> buf;
};

} // namespace Comm
} // namespace Ume

#endif
