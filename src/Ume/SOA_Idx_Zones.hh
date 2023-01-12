/*
\file SOA_Idx_Zones.hh
*/

#ifndef SOA_IDX_ZONES_HH
#define SOA_IDX_ZONES_HH 1

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
  class DSE_zcoord : public DSE_Base<Zones> {
  public:
    explicit DSE_zcoord(Zones &z) : DSE_Base(Types::VEC3V, z) {}

  protected:
    bool init_() const override;
  };
};

} // namespace SOA_Idx
} // namespace Ume
#endif
