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
  \file Ume/SOA_Entity.hh
*/
#ifndef UME_SOA_ENTITY_HH
#define UME_SOA_ENTITY_HH 1

#include "Ume/Comm_Neighbors.hh"
#include "Ume/Comm_Transport.hh"
#include "Ume/Datastore.hh"
#include <iosfwd>
#include <ranges>
#include <string>
#include <vector>

namespace Ume {
namespace SOA_Idx {

struct Mesh;

//! Record information common to all SOA_Idx::Mesh entities
struct Entity {
  Entity() = delete;
  explicit Entity(Mesh *mesh) : mesh_{mesh} {}
  /* An Entity caches the Mesh it was constructed against, so a copy or a move
     would silently keep pointing at the original: `auto z = mesh.zones;` would
     hand back something whose ds() and comm() resolve to another mesh.  This is
     also what makes Mesh non-relocatable -- the deletions there are the same
     invariant stated where a reader of Mesh will meet it. */
  Entity(Entity const &) = delete;
  Entity &operator=(Entity const &) = delete;
  Entity(Entity &&) = delete;
  Entity &operator=(Entity &&) = delete;
  enum CommTypes {
    INTERNAL = 1, //!< Not on a communication boundary
    SOURCE, //!< The source entity in a group of shared copies
    COPY, //!< Non-source entity in a group of shared copies
    GHOST //!< A non-shared ghost copy of a remote entity
  };

  //! Mask flag
  std::vector<short> mask;

  //! The communication type for this entity
  std::vector<int> comm_type;

  /*! @name Minimal parallel connectivity

    These arrays define minimum inter-mesh connectivity.  Each entity listed in
    `cpy_idx` is a copy of an entity in a different rank.  The pair `{src_pe[i],
    src_idx[i]}` describes the address of the source entity for this copy.

    Copies of volumetric entries are called "ghosts", and they are generally not
    iterated across in a calculation, but are referenced via local connectivity.
    The source for a ghost is called a "real" entity. To simplify iteration, all
    ghosts are stored in the upper portion of the Entity indices: [lsize,
    size). Generally, ghost entities are only updated through scatter
    operations from sources to copies.

    Entities that are shared across processor boundaries are stored among the
    real entries in the index range [0, lsize).  The instance of a shared entity
    on some processor is designated the source; the rest are copies.  Some
    shared entities, such as faces, have one-to-one connectivity: for each
    source, there is one copy.  Other shared entities, such as points, are
    one-to-many: each source may have multiple copies. This occurs when an
    entity appears in multiple processor mesh domains.  Generally, all
    processors compute on shared entities, which are then combined with a
    gather-scatter paradigm, or simply a scatter from the src to ensure each
    partition is operating on the same value.

    This minimal connectivity is expressed in one direction: each copy knows
    where its source entity is.  This allows a simple representation, as this
    direction is always one-to-one.  But this means that a PE does not know what
    source entities reside locally, they only know the copies!
   */
  ///@{
  //! Local indices of copies
  std::vector<int> cpy_idx;
  //! The rank that owns the source entity
  std::vector<int> src_pe;
  //! The index of the source entity on the src_pe
  std::vector<int> src_idx;
  //! The type of ghost
  std::vector<int> ghost_mask;
  ///@}

  //! Local entity indices that are copies of source entities on a source PE
  /*! myCpys[i].elements[j] is the (local) entity index corresponding to the
      j'th entry in the buffer sent to/from the source PE myCpys[i].pe.  These
      are sent to the source PE during a gather, and received during a scatter.
   */
  Ume::Comm::Neighbors myCpys;
  //! Local entity indices that are sources for copy entities on copy PEs.
  /*! mySrcs[i].elements[j] is the (local) entity index corresponding to the
      j'th entry in the buffer sent to/from the source PE mySrcs[i].pe.  These
      are received from the copy PE during a gather, and sent to the copy PEs
      during a scatter.
   */
  Ume::Comm::Neighbors mySrcs;

  //! Do a remote gather for a `field` on this entity, combined with `op`
  /*! Leaves `field` unchanged when the transport does not exchange
      (Comm::Transport::does_exchanges() is false, as for Dummy_Transport). */
  template <typename FT> void gather(Comm::Op const op, FT &field);

  //! Do a scatter to remotes for a `field` on this entity
  /*! Leaves `field` unchanged when the transport does not exchange
      (Comm::Transport::does_exchanges() is false, as for Dummy_Transport). */
  template <typename FT> void scatter(FT &field);

  //! Combined gather-scatter operation
  /*! Leaves `field` unchanged when the transport does not exchange
      (Comm::Transport::does_exchanges() is false, as for Dummy_Transport). */
  template <typename FT> void gathscat(Comm::Op const op, FT &field);

  //! Define a named subset of this Entity's elements
  struct Subset {
    std::string name;
    //! The Number of local (non-ghost) elements
    int lsize = 0;
    std::vector<int> elements;
    std::vector<short> mask;
    inline bool operator==(Subset const &rhs) const {
      return (rhs.name == name && rhs.lsize == lsize &&
          rhs.elements == elements && rhs.mask == mask);
    }
  };

  //! The subsets defined on this Entity
  std::vector<Subset> subsets;

  /* Element size queries. */
  //! Return the number of elements in this Entity.
  constexpr int size() const { return static_cast<int>(mask.size()); }
  //! Return the number of non-ghost elements in this Entity.
  constexpr int local_size() const { return lsize_; }
  //! Return the number of ghost elements in this Entity.
  constexpr int ghost_local_size() const { return size() - lsize_; }
  //! Return the number of ghost elements in this Entity, which may be zero.
  constexpr int ghost_size() const { return ghost_local_size(); }

  /* Element sequences for iterating over.  Each is half-open, and empty when
     there are no such elements. */
  //! Return a sequence over all indices, [0, size()).
  constexpr auto all_indices() const {
    return std::ranges::iota_view{0, size()};
  }
  //! Return a sequence over non-ghost indices, [0, local_size()).
  constexpr auto local_indices() const {
    return std::ranges::iota_view{0, lsize_};
  }
  //! Return a sequence over ghost indices, [local_size(), size()).
  constexpr auto ghost_indices() const {
    return std::ranges::iota_view{lsize_, size()};
  }
  //! Return a sequence over ghost indices offset to 0, [0, ghost_size()).
  constexpr auto ghost_indices_offset() const {
    return std::ranges::iota_view{0, ghost_size()};
  }

  virtual void write(std::ostream &os) const = 0;
  virtual void read(std::istream &is) = 0;
  virtual void resize(int const local, int const total, int const ghost);
  bool operator==(Entity const &rhs) const;

  /* ds() and comm() are defined in SOA_Entity.cc, where Mesh is complete:
     here it is only forward-declared, and a cast to its Mesh_Base base would
     have to be a reinterpret_cast that ignores the base's offset. */
  //! Return the Datastore of the mesh that this Entity belongs to
  Datastore &ds();
  //! Return the Datastore of the mesh that this Entity belongs to (const)
  Datastore const &ds() const;
  //! Return the Mesh that this Entity belongs to
  constexpr Mesh &mesh() { return *mesh_; }
  //! Return the Mesh that this Entity belongs to (const)
  constexpr Mesh const &mesh() const { return *mesh_; }
  //! Return the Transport that this Entity belongs to
  Ume::Comm::Transport &comm();
  //! Return the Transport that this Entity belongs to (const)
  Ume::Comm::Transport const &comm() const;

private:
  Mesh *mesh_;
  //! The number of local (non-ghost) entities
  int lsize_ = 0;
};

} // namespace SOA_Idx

/* Declared here so that a caller outside SOA_Entity.cc reaches these rather
   than instantiating the generic std::vector overloads, which would copy the
   Subset objects' bytes. */
//! Binary write specialization for std::vector<Entity::Subset>
template <>
void write_bin<SOA_Idx::Entity::Subset>(
    std::ostream &os, std::vector<SOA_Idx::Entity::Subset> const &data);

//! Binary read specialization for std::vector<Entity::Subset>
template <>
void read_bin<SOA_Idx::Entity::Subset>(
    std::istream &is, std::vector<SOA_Idx::Entity::Subset> &data);

} // namespace Ume

#endif
