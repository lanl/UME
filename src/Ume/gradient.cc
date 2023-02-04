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

  pvol.assign(pll, 1.0e-90);
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
} // namespace Ume
