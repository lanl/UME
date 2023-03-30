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
\file Ume/SOA_Idx_Edges.hh
*/

#ifndef UME_SOA_IDX_EDGES_HH
#define UME_SOA_IDX_EDGES_HH 1

#include "Ume/Entity_Field.hh"
#include "Ume/SOA_Entity.hh"

namespace Ume {
namespace SOA_Idx {

//! SoA representation of mesh edges (connects two points)
struct Edges : public Entity {
  explicit Edges(Mesh *mesh);
  void write(std::ostream &os) const override;
  void read(std::istream &is) override;
  void resize(int const local, int const total, int const ghost) override;
  bool operator==(Edges const &rhs) const;

  //! Edge field variable: edge centers
  class VAR_ecoord : public Entity_Field<Edges> {
  public:
    explicit VAR_ecoord(Edges &e) : Entity_Field(Types::VEC3V, e) {}

  protected:
    bool init_() const override;
  };
};

} // namespace SOA_Idx
} // namespace Ume
#endif
