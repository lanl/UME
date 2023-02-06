/*!
  \file Ume/gradient.cc
*/

#include "Ume/gradient.hh"

namespace Ume {

using DBLV_T = DS_Types::DBLV_T;
using VEC3V_T = DS_Types::VEC3V_T;
using VEC3_T = DS_Types::VEC3_T;

void gradzatp(Ume::SOA_Idx::Mesh &mesh, DBLV_T const &zfield, VEC3V_T &pgrad,
    DBLV_T &pvol) {
  auto const &csurf = mesh.ds->caccess_vec3v("corner_csurf");
  auto const &cv = mesh.ds->caccess_dblv("corner_vol");
  auto const &pnorm = mesh.ds->caccess_vec3v("point_norm");
  auto const &kkcp = mesh.ds->caccess_intv("m:c>p");
  auto const &kkcz = mesh.ds->caccess_intv("m:c>z");
  auto const &kctyp = mesh.corners.mask;
  auto const &kptyp = mesh.points.mask;

  int const pll = mesh.points.size();
  int const pl = mesh.points.lsize;
  int const cl = mesh.corners.lsize;

  pvol.assign(pll, 0.0);
  pgrad.assign(pll, VEC3_T(0.0));

  for (int c = 0; c < cl; ++c) {
    if (kctyp[c] < 1)
      continue; // Only operate on interior corners
    int const z = kkcz[c];
    int const p = kkcp[c];
    pvol[p] += cv[c];
    pgrad[p] += csurf[c] * zfield[z];
  }

  mesh.points.gathscat(Ume::Comm::Op::SUM, pvol);
  mesh.points.gathscat(Ume::Comm::Op::SUM, pgrad);

  /*
    Divide by point control volume to get gradient.  If a point is on the outer
    perimeter of the mesh (KPTYP=-1), subtract the outward normal component of
    the gradient using the point normals (PNORM).
   */
  for (int p = 0; p < pl; ++p) {
    if (kptyp[p] > 0) {
      // Internal point
      pgrad[p] /= pvol[p];
    } else if (kptyp[p] == -1) {
      // Mesh boundary point
      double const ppdot = dotprod(pgrad[p], pnorm[p]);
      pgrad[p] = (pgrad[p] - pnorm[p] * ppdot) / pvol[p];
    }
  }
}

void gradzatz(Ume::SOA_Idx::Mesh &mesh, DBLV_T const &zone_field,
    VEC3V_T &point_gradient, VEC3V_T &zone_gradient) {
  auto const &c_to_z_map = mesh.ds->caccess_intv("m:c>z");
  auto const &c_to_p_map = mesh.ds->caccess_intv("m:c>p");
  int const num_local_corners = mesh.corners.lsize;
  auto const &corner_type = mesh.corners.mask;
  auto const &corner_volume = mesh.ds->caccess_dblv("corner_vol");

  // Get the control volume and the field gradient at each mesh point.
  DBLV_T point_volume;
  gradzatp(mesh, zone_field, point_gradient, point_volume);

  // Accumulate the zone volume
  DBLV_T zone_volume(mesh.zones.size(), 0.0);
  for (int corner_idx = 0; corner_idx < num_local_corners; ++corner_idx) {
    if (corner_type[corner_idx] < 1)
      continue; // Only operate on interior corners
    int const zone_idx = c_to_z_map[corner_idx];
    zone_volume[zone_idx] += corner_volume[corner_idx];
  }

  // Accumulate the zone-centered gradient
  zone_gradient.assign(mesh.zones.size(), VEC3_T(0.0));
  for (int corner_idx = 0; corner_idx < num_local_corners; ++corner_idx) {
    if (corner_type[corner_idx] < 1)
      continue; // Only operate on interior corners
    int const zone_idx = c_to_z_map[corner_idx];
    int const point_idx = c_to_p_map[corner_idx];
    double const c_z_vol_ratio =
        corner_volume[corner_idx] / zone_volume[zone_idx];
    zone_gradient[zone_idx] += point_gradient[point_idx] * c_z_vol_ratio;
  }
}

} // namespace Ume
