/*!
  \file Ume/SOA_Idx_Sides.hh
*/

#ifndef UME_SOA_IDX_SIDES_HH
#define UME_SOA_IDX_SIDES_HH 1

#include "Ume/Entity_Field.hh"
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

  //! Side field variable: area-weighted normal of corner facet
  /*! The two corners that intersect this side do so on the plane of the
      triangular facet formed from the centers of the zone, face, and edge which
      bisects this side. This variable contains the area-weighted normal of that
      facet for each side.  The corners will sum this vector across all sides to
      produce a area-weighted surface normal. */
  class VAR_side_surf : public Entity_Field<Sides> {
  public:
    explicit VAR_side_surf(Sides &s) : Entity_Field(Types::VEC3V, s) {}

  protected:
    bool init_() const override;
  };

  //! Side field variable: area-weighted normal of zone facet
  /*! Similar to VAR_side_surf, this contains the area-weighted normal of the
      facet that separates zones, defined by the side edge and the center of the
      side face. */
  class VAR_side_surz : public Entity_Field<Sides> {
  public:
    explicit VAR_side_surz(Sides &s) : Entity_Field(Types::VEC3V, s) {}

  protected:
    bool init_() const override;
  };

  //! Side field variable: volume of the side
  class VAR_side_vol : public Entity_Field<Sides> {
  public:
    explicit VAR_side_vol(Sides &s) : Entity_Field(Types::DBLV, s) {}

  protected:
    bool init_() const override;
  };
};

} // namespace SOA_Idx
} // namespace Ume
#endif
