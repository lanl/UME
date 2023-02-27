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

  // The center of each zone
  class DSE_zcoord : public Entity_Field<Zones> {
  public:
    explicit DSE_zcoord(Zones &z) : Entity_Field(Types::VEC3V, z) {}

  protected:
    bool init_() const override;
  };

  // The point-connected zone neighbors
  class DSE_zone_to_pt_zone : public Entity_Field<Zones> {
  public:
    explicit DSE_zone_to_pt_zone(Zones &z) : Entity_Field(Types::INTRR, z) {}

  protected:
    bool init_() const override;
  };

  // The points for each zone
  class DSE_zone_to_points : public Entity_Field<Zones> {
  public:
    explicit DSE_zone_to_points(Zones &z) : Entity_Field(Types::INTRR, z) {}

  protected:
    bool init_() const override;
  };
};

} // namespace SOA_Idx
} // namespace Ume
#endif
