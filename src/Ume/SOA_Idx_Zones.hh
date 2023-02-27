/*
\file SOA_Idx_Zones.hh
*/

#ifndef UME_SOA_IDX_ZONES_HH
#define UME_SOA_IDX_ZONES_HH 1

#include "Ume/DSE_Base.hh"
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
  class DSE_zcoord : public DSE_Base<Zones> {
  public:
    explicit DSE_zcoord(Zones &z) : DSE_Base(Types::VEC3V, z) {}

  protected:
    bool init_() const override;
  };

  // The point-connected zone neighbors
  class DSE_zone_to_pt_zone : public DSE_Base<Zones> {
  public:
    explicit DSE_zone_to_pt_zone(Zones &z) : DSE_Base(Types::INTRR, z) {}

  protected:
    bool init_() const override;
  };

  // The points for each zone
  class DSE_zone_to_points : public DSE_Base<Zones> {
  public:
    explicit DSE_zone_to_points(Zones &z) : DSE_Base(Types::INTRR, z) {}

  protected:
    bool init_() const override;
  };
};

} // namespace SOA_Idx
} // namespace Ume
#endif
