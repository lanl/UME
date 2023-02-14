/*
\file SOA_Idx_Points.hh
*/

#ifndef SOA_IDX_POINTS_HH
#define SOA_IDX_POINTS_HH 1

#include "Ume/DSE_Base.hh"
#include "Ume/SOA_Entity.hh"

namespace Ume {
namespace SOA_Idx {

using PtCoord = Vec3;

//! SoA representation of mesh points (connects two points)
struct Points : public Entity {
  explicit Points(Mesh *mesh);
  void write(std::ostream &os) const override;
  void read(std::istream &is) override;
  void resize(int const local, int const total, int const ghost) override;
  bool operator==(Points const &rhs) const;

  class DSE_point_to_zones : public DSE_Base<Points> {
  public:
    explicit DSE_point_to_zones(Points &p) : DSE_Base(Types::INTRR, p) {}

  protected:
    bool init_() const override;
  };

  class DSE_point_norm : public DSE_Base<Points> {
  public:
    explicit DSE_point_norm(Points &p) : DSE_Base(Types::VEC3V, p) {}

  protected:
    bool init_() const override;
  };
};

} // namespace SOA_Idx
} // namespace Ume
#endif
