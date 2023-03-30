/*
  Copyright (c) 2023, Triad National Security, LLC. All rights reserved.

  This is open source software; you can redistribute it and/or modify it under
  the terms of the BSD-3 License. If software is modified to produce derivative
  works, such modified software should be clearly marked, so as not to confuse
  it with the version available from LANL. Full text of the BSD-3 License can be
  found in the LICENSE.md file, and the full assertion of copyright in the
  NOTICE.md file.
*/

/*
\file Ume/SOA_Idx_Zones.hh
*/

#ifndef UME_SOA_IDX_ZONES_HH
#define UME_SOA_IDX_ZONES_HH 1

#include "Ume/Entity_Field.hh"
#include "Ume/SOA_Entity.hh"

namespace Ume {
namespace SOA_Idx {

//! SoA representation of mesh zones
struct Zones : public Entity {
  explicit Zones(Mesh *mesh);
  void write(std::ostream &os) const override;
  void read(std::istream &is) override;
  void resize(int const local, int const total, int const ghost) override;
  bool operator==(Zones const &rhs) const;

  //! Zone field variable: the center point of each zone
  class VAR_zcoord : public Entity_Field<Zones> {
  public:
    explicit VAR_zcoord(Zones &z) : Entity_Field(Types::VEC3V, z) {}

  protected:
    bool init_() const override;
  };

  //! Zone field variable: point-connected zone neighbors inverse connectivity
  class VAR_zone_to_pt_zone : public Entity_Field<Zones> {
  public:
    explicit VAR_zone_to_pt_zone(Zones &z) : Entity_Field(Types::INTRR, z) {}

  protected:
    bool init_() const override;
  };

  // Zone field variable: zone-to-points inverse connectivity
  class VAR_zone_to_points : public Entity_Field<Zones> {
  public:
    explicit VAR_zone_to_points(Zones &z) : Entity_Field(Types::INTRR, z) {}

  protected:
    bool init_() const override;
  };

  // Zone field variable: zone-to-corners inverse connectivity
  class VAR_zone_to_corners : public Entity_Field<Zones> {
  public:
    explicit VAR_zone_to_corners(Zones &z) : Entity_Field(Types::INTRR, z) {}

  protected:
    bool init_() const override;
  };
};

} // namespace SOA_Idx
} // namespace Ume
#endif
