/*!
  \file SOA_Entity.hh
*/
#ifndef SOA_ENTITY_HH
#define SOA_ENTITY_HH 1

#include <iosfwd>
#include <string>
#include <vector>

namespace Ume {
namespace SOA_Idx {

/*! Record information common to all SOA_Idx::Mesh entities */
struct Entity {
  enum CommTypes {
    INTERNAL = 1, /* Not on a communication boundary */
    SOURCE, /* The source entity in a group of shared copies */
    COPY, /* Non-source entity in a group of shared copies */
    GHOST /* A non-shared ghost copy of a remote entity */
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

  struct Comm {
    int pe;
    std::vector<int> elements;
    inline bool operator==(Comm const &rhs) const {
      return (pe == rhs.pe && elements == rhs.elements);
    }
  };
  std::vector<Comm> recvFrom;
  std::vector<Comm> sendTo;

  struct Subset {
    std::string name;
    std::vector<int> elements;
    std::vector<short> mask;
    inline bool operator==(Subset const &rhs) const {
      return (rhs.name == name && rhs.elements == elements && rhs.mask == mask);
    }
  };
  std::vector<Subset> subsets;

  //! The number of local (non-ghost) entities
  int lsize = 0;

  virtual void write(std::ostream &os) const = 0;
  virtual void read(std::istream &is) = 0;
  virtual void resize(int const local, int const total, int const ghost);
  bool operator==(Entity const &rhs) const;
  int size() const { return static_cast<int>(mask.size()); }
};
} // namespace SOA_Idx
} // namespace Ume

#endif