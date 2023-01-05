/*
\file SOA_Idx_Corners.hh
*/

#ifndef SOA_IDX_CORNERS_HH
#define SOA_IDX_CORNERS_HH 1

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
  void write(dsptr ds, std::ostream &os) const;
  void read(std::istream &is) override;
  void resize(int const local, int const total, int const ghost) override;
  bool operator==(Corners const &rhs) const;

  class DSE_corner_vol : public DS_Entry {
  public:
    explicit DSE_corner_vol(Corners &c) : DS_Entry(Types::DBLV), corners_{c} {}

  protected:
    void init_() const override;

  private:
    Corners const &corners_;
  };
};

} // namespace SOA_Idx
} // namespace Ume
#endif
