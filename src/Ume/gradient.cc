/*!
  \file Ume/gradient.cc
*/
#include "Ume/gradient.hh"

namespace Ume {

using DBLV_T = DS_Types::DBLV_T;
using VEC3V_T = DS_Types::VEC3V_T;
using VEC3_T = DS_Types::VEC3_T;

void gradzatp(Ume::SOA_Idx::Mesh &mesh, DBLV_T const &zone_field,
    VEC3V_T &point_gradient) {
  auto const &csurf = mesh.ds->caccess_vec3v("corner_csurf");
  auto const &corner_volume = mesh.ds->caccess_dblv("corner_vol");
  auto const &point_normal = mesh.ds->caccess_vec3v("point_norm");
  auto const &c_to_p_map = mesh.ds->caccess_intv("m:c>p");
  auto const &c_to_z_map = mesh.ds->caccess_intv("m:c>z");
  auto const &corner_type = mesh.corners.mask;
  auto const &point_type = mesh.points.mask;

  int const pll = mesh.points.size();
  int const pl = mesh.points.local_size();
  int const cl = mesh.corners.local_size();

  DBLV_T point_volume(pll, 0.0);
  point_gradient.assign(pll, VEC3_T(0.0));

  for (int c = 0; c < cl; ++c) {
    if (corner_type[c] < 1)
      continue; // Only operate on interior corners
    int const z = c_to_z_map[c];
    int const p = c_to_p_map[c];
    point_volume[p] += corner_volume[c];
    point_gradient[p] += csurf[c] * zone_field[z];
  }

  mesh.points.gathscat(Ume::Comm::Op::SUM, point_volume);
  mesh.points.gathscat(Ume::Comm::Op::SUM, point_gradient);

  /*
    Divide by point control volume to get gradient.  If a point is on the outer
    perimeter of the mesh (POINT_TYPE=-1), subtract the outward normal component
    of the gradient using the point normals.
   */
  for (int p = 0; p < pl; ++p) {
    if (point_type[p] > 0) {
      // Internal point
      point_gradient[p] /= point_volume[p];
    } else if (point_type[p] == -1) {
      // Mesh boundary point
      double const ppdot = dotprod(point_gradient[p], point_normal[p]);
      point_gradient[p] =
          (point_gradient[p] - point_normal[p] * ppdot) / point_volume[p];
    }
  }
  mesh.points.scatter(point_gradient);
}

void gradzatz(Ume::SOA_Idx::Mesh &mesh, DBLV_T const &zone_field,
    VEC3V_T &zone_gradient, VEC3V_T &point_gradient) {
  auto const &c_to_z_map = mesh.ds->caccess_intv("m:c>z");
  auto const &c_to_p_map = mesh.ds->caccess_intv("m:c>p");
  int const num_local_corners = mesh.corners.local_size();
  auto const &corner_type = mesh.corners.mask;
  auto const &corner_volume = mesh.ds->caccess_dblv("corner_vol");

  // Get the field gradient at each mesh point.
  gradzatp(mesh, zone_field, point_gradient);

  /* Accumulate the zone volume.  Note that we need to allocate a zone field for
     volume, as we are accumulating from corners */
  DBLV_T zone_volume(mesh.zones.size(), 0.0);
  for (int corner_idx = 0; corner_idx < num_local_corners; ++corner_idx) {
    if (corner_type[corner_idx] < 1)
      continue; // Only operate on interior corners
    int const zone_idx = c_to_z_map[corner_idx];
    /* Note that we cannot parallelize across corners, as multiple corners
       write to the same zone. */
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

  mesh.zones.scatter(zone_gradient);
}

void gradzatp_invert(Ume::SOA_Idx::Mesh &mesh, DBLV_T const &zone_field,
    VEC3V_T &point_gradient) {
  auto const &csurf = mesh.ds->caccess_vec3v("corner_csurf");
  auto const &corner_volume = mesh.ds->caccess_dblv("corner_vol");
  auto const &point_normal = mesh.ds->caccess_vec3v("point_norm");
  auto const &p_to_c_map = mesh.ds->caccess_intrr("m:p>rc");
  auto const &c_to_z_map = mesh.ds->caccess_intv("m:c>z");
  auto const &point_type = mesh.points.mask;

  int const num_points = mesh.points.size();
  int const num_local_points = mesh.points.local_size();

  DBLV_T point_volume(num_points, 0.0);
  point_gradient.assign(num_points, VEC3_T(0.0));

  for (int point_idx = 0; point_idx < num_local_points; ++point_idx) {
    for (int const &corner_idx : p_to_c_map[point_idx]) {
      int const zone_idx = c_to_z_map[corner_idx];
      point_volume[point_idx] += corner_volume[corner_idx];
      point_gradient[point_idx] += csurf[corner_idx] * zone_field[zone_idx];
    }
  }

  mesh.points.gathscat(Ume::Comm::Op::SUM, point_volume);
  mesh.points.gathscat(Ume::Comm::Op::SUM, point_gradient);

  /*
    Divide by point control volume to get gradient.  If a point is on the outer
    perimeter of the mesh (POINT_TYPE=-1), subtract the outward normal component
    of the gradient using the point normals.
   */
  for (int point_idx = 0; point_idx < num_local_points; ++point_idx) {
    if (point_type[point_idx] > 0) {
      // Internal point
      point_gradient[point_idx] /= point_volume[point_idx];
    } else if (point_type[point_idx] == -1) {
      // Mesh boundary point
      double const ppdot =
          dotprod(point_gradient[point_idx], point_normal[point_idx]);
      point_gradient[point_idx] =
          (point_gradient[point_idx] - point_normal[point_idx] * ppdot) /
          point_volume[point_idx];
    }
  }
  mesh.points.scatter(point_gradient);
}

void gradzatz_invert(Ume::SOA_Idx::Mesh &mesh, DBLV_T const &zone_field,
    VEC3V_T &zone_gradient, VEC3V_T &point_gradient) {
  auto const &z_to_c_map = mesh.ds->caccess_intrr("m:z>c");
  auto const &c_to_p_map = mesh.ds->caccess_intv("m:c>p");
  int const num_local_zones = mesh.zones.local_size();
  auto const &zone_type = mesh.zones.mask;
  auto const &corner_volume = mesh.ds->caccess_dblv("corner_vol");

  // Get the field gradient at each mesh point.
  gradzatp_invert(mesh, zone_field, point_gradient);

  zone_gradient.assign(mesh.zones.size(), VEC3_T(0.0));
  for (int zone_idx = 0; zone_idx < num_local_zones; ++zone_idx) {
    if (zone_type[zone_idx] < 1)
      continue; // Only operate on local interior zones

    // Accumulate the (local) zone volume
    double zone_volume{0.0}; // Only need a local volume
    for (int const &corner_idx : z_to_c_map[zone_idx]) {
      zone_volume += corner_volume[corner_idx];
    }

    for (int const &corner_idx : z_to_c_map[zone_idx]) {
      int const point_idx = c_to_p_map[corner_idx];
      double const c_z_vol_ratio = corner_volume[corner_idx] / zone_volume;
      zone_gradient[zone_idx] += point_gradient[point_idx] * c_z_vol_ratio;
    }
  }

  mesh.zones.scatter(zone_gradient);
}

} // namespace Ume
