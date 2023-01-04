/*!
  \file SOA_Idx_Sides.hh
*/

#ifndef SOA_IDX_SIDES_HH
#define SOA_IDX_SIDES_HH 1

#include "Ume/SOA_Entity.hh"

namespace Ume {
namespace SOA_Idx {
//! SoA representation of mesh sides
/*! A side is another subzonal quantity, formed by a zone centroid,
    the centroid of a face on that zone, and an edge on that face.  On
    a hexahedral mesh, a side is a tetrahedron.  The side is the
    principal entity for volumetric calculations, so there is a lot of
    additional connectivity information carried here. */
struct Sides : public Entity {
  explicit Sides(Mesh *mesh);
  void write(std::ostream &os) const override;
  void read(std::istream &is) override;
  void resize(int const local, int const total, int const ghost) override;
  bool operator==(Sides const &rhs) const;

  class DSE_side_vol : public DS_Entry {
  public:
    explicit DSE_side_vol(Sides &s) : DS_Entry(Types::DBLV), sides_{s} {}

  protected:
    void init_() const override;

  private:
    Sides const &sides_;
  };
};

} // namespace SOA_Idx
} // namespace Ume
#endif
