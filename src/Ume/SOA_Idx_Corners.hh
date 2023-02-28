/*!
\file Ume/SOA_Idx_Corners.hh
*/

#ifndef UME_SOA_IDX_CORNERS_HH
#define UME_SOA_IDX_CORNERS_HH 1

#include "Ume/Entity_Field.hh"
#include "Ume/SOA_Entity.hh"

namespace Ume {
namespace SOA_Idx {

//! SoA representation of mesh corners
/*!
  A corner is a subzonal volume defined by a zone centroid, a point
  on that zone, and the midpoints of the zone faces adjacent to that
  point, and the midpoints of the zone edges adjacent to that point.
  On a hexahedral mesh, a corner is a hexagon as well.
*/
struct Corners : public Entity {
public:
  Corners() = delete;
  explicit Corners(Mesh *mesh);
  void write(std::ostream &os) const override;
  void read(std::istream &is) override;
  void resize(int const local, int const total, int const ghost) override;
  bool operator==(Corners const &rhs) const;

  //! Corner field variable: corner volume
  class VAR_corner_vol : public Entity_Field<Corners> {
  public:
    explicit VAR_corner_vol(Corners &c) : Entity_Field(Types::DBLV, c) {}

  protected:
    bool init_() const override;
  };

  //! Corner field variable: sum of area-weighted face normals
  class VAR_corner_csurf : public Entity_Field<Corners> {
  public:
    explicit VAR_corner_csurf(Corners &c) : Entity_Field(Types::VEC3V, c) {}

  protected:
    bool init_() const override;
  };

  //! Corner field variable: corner-to-sides inverse connectivity map
  class VAR_corner_to_sides : public Entity_Field<Corners> {
  public:
    explicit VAR_corner_to_sides(Corners &c) : Entity_Field(Types::INTRR, c) {}

  protected:
    bool init_() const override;
  };
};

} // namespace SOA_Idx
} // namespace Ume
#endif
