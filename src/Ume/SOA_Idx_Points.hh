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
\file Ume/SOA_Idx_Points.hh
*/

#ifndef UME_SOA_IDX_POINTS_HH
#define UME_SOA_IDX_POINTS_HH 1

#include "Ume/Entity_Field.hh"
#include "Ume/SOA_Entity.hh"

namespace Ume {
namespace SOA_Idx {

using PtCoord = Vec3;

//! SoA representation of mesh points
struct Points : public Entity {
  explicit Points(Mesh *mesh);
  void write(std::ostream &os) const override;
  void read(std::istream &is) override;
  void resize(int const local, int const total, int const ghost) override;
  bool operator==(Points const &rhs) const;

  //! Point field variable: point-to-zones inverse connectivity map
  class VAR_point_to_zones : public Entity_Field<Points> {
  public:
    explicit VAR_point_to_zones(Points &p) : Entity_Field(Types::INTRR, p) {}

  protected:
    bool init_() const override;
  };

  //! Point field variable: point-to-real corners inverse connectivity map
  class VAR_point_to_real_corners : public Entity_Field<Points> {
  public:
    explicit VAR_point_to_real_corners(Points &p)
        : Entity_Field(Types::INTRR, p) {}

  protected:
    bool init_() const override;
  };

  //! Point field variable: sum of adjacent VAR_side_surz
  class VAR_point_norm : public Entity_Field<Points> {
  public:
    explicit VAR_point_norm(Points &p) : Entity_Field(Types::VEC3V, p) {}

  protected:
    bool init_() const override;
  };
};

} // namespace SOA_Idx
} // namespace Ume
#endif
