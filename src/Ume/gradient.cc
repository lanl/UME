/*!
  \file Ume/gradient.cc
*/
#include "Ume/gradient.hh"

#include <fstream>
#include <iomanip>

namespace Ume {

using DBLV_T = DS_Types::DBLV_T;
using VEC3V_T = DS_Types::VEC3V_T;
using VEC3_T = DS_Types::VEC3_T;

#ifdef USE_SCORIA
void gradzatp(struct client *client, Ume::SOA_Idx::Mesh &mesh,
    DBLV_T const &zone_field, VEC3V_T &point_gradient) {
#else
void gradzatp(Ume::SOA_Idx::Mesh &mesh, DBLV_T const &zone_field,
    VEC3V_T &point_gradient) {
#endif

  auto const &csurf = mesh.ds->caccess_vec3v("corner_csurf");
  auto const &corner_volume = mesh.ds->caccess_dblv("corner_vol");
  auto const &point_normal = mesh.ds->caccess_vec3v("point_norm");
  auto const &c_to_p_map = mesh.ds->caccess_intv("m:c>p");
  auto const &c_to_z_map = mesh.ds->caccess_intv("m:c>z");
  auto const &corner_type = mesh.corners.mask;
  auto const &point_type = mesh.points.mask;

#ifdef USE_SCORIA
  const UmeVector<size_t> mc_to_z_map(begin(c_to_z_map), end(c_to_z_map));
  const UmeVector<size_t> mc_to_p_map(begin(c_to_p_map), end(c_to_p_map));
#endif

  int const pll = mesh.points.size();
  int const pl = mesh.points.lsize;
  int const cl = mesh.corners.lsize;

  DBLV_T point_volume(pll, 0.0);
  point_gradient.assign(pll, VEC3_T(0.0));

#ifdef USE_SCORIA
  DBLV_T packed_zf(cl, 0.0);

  struct request req1;
  scoria_read(client, zone_field.data(), cl, packed_zf.data(),
      mc_to_z_map.data(), NULL, 0, NONE, &req1);
  wait_request(client, &req1);

  DBLV_T packed_pv(cl, 0.0);

  VEC3V_T packed_pg(cl);
  packed_pg.assign(cl, VEC3_T(0.0));

  for (int c = 0; c < cl; ++c) {
    if (corner_type[c] < 1)
      continue; // Only operate on interiod corners
    packed_pv[c] = corner_volume[c];
    packed_pg[c] = csurf[c] * packed_zf[c];
  }

  struct request req2;
  scoria_write(client, point_volume.data(), cl, packed_pv.data(),
      mc_to_p_map.data(), NULL, 0, NONE, &req2);
  wait_request(client, &req2);

  struct request req3;
  scoria_write(client, point_gradient.data(), cl, packed_pg.data(),
      mc_to_p_map.data(), NULL, 0, NONE, &req3);
  wait_request(client, &req3);

#else
  for (int c = 0; c < cl; ++c) {
    if (corner_type[c] < 1)
      continue; // Only operate on interior corners
    int const z = c_to_z_map[c];
    int const p = c_to_p_map[c];
    point_volume[p] += corner_volume[c];
    point_gradient[p] += csurf[c] * zone_field[z];
  }
#endif
  std::ofstream fpv("point_volume.txt");
  fpv << std::setprecision(10);

  for (auto const &x : point_volume)
    fpv << x << '\n';

  fpv.close();

  std::ofstream fpg("point_gradient.txt");
  fpg << std::setprecision(10);

  for (auto const &x : point_gradient)
    fpg << x << '\n';

  fpg.close();

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

#ifdef USE_SCORIA
void gradzatz(struct client *client, Ume::SOA_Idx::Mesh &mesh,
    DBLV_T const &zone_field, VEC3V_T &zone_gradient, VEC3V_T &point_gradient) {
#else
void gradzatz(Ume::SOA_Idx::Mesh &mesh, DBLV_T const &zone_field,
    VEC3V_T &zone_gradient, VEC3V_T &point_gradient) {
#endif

  auto const &c_to_z_map = mesh.ds->caccess_intv("m:c>z");
  auto const &c_to_p_map = mesh.ds->caccess_intv("m:c>p");
  int const num_local_corners = mesh.corners.lsize;
  auto const &corner_type = mesh.corners.mask;
  auto const &corner_volume = mesh.ds->caccess_dblv("corner_vol");

#ifdef USE_SCORIA
  const UmeVector<size_t> mc_to_z_map(begin(c_to_z_map), end(c_to_z_map));
  const UmeVector<size_t> mc_to_p_map(begin(c_to_p_map), end(c_to_p_map));
#endif

  // Get the field gradient at each mesh point.
#ifdef USE_SCORIA
  gradzatp(client, mesh, zone_field, point_gradient);
#else
  gradzatp(mesh, zone_field, point_gradient);
#endif

  // Accumulate the zone volume
  DBLV_T zone_volume(mesh.zones.size(), 0.0);
#ifdef USE_SCORIA
  DBLV_T packed_zv(num_local_corners, 0.0);

  for (int corner_idx = 0; corner_idx < num_local_corners; ++corner_idx) {
    if (corner_type[corner_idx] < 1)
      continue; // Only operate on interior corners
    packed_zv[corner_idx] = corner_volume[corner_idx];
  }

  struct request req1;
  scoria_write(client, zone_volume.data(), num_local_corners, packed_zv.data(),
      mc_to_z_map.data(), NULL, 0, NONE, &req1);
  wait_request(client, &req1);
#else
  for (int corner_idx = 0; corner_idx < num_local_corners; ++corner_idx) {
    if (corner_type[corner_idx] < 1)
      continue; // Only operate on interior corners
    int const zone_idx = c_to_z_map[corner_idx];
    zone_volume[zone_idx] += corner_volume[corner_idx];
  }
#endif
  std::ofstream fzv("zone_volume.txt");
  fzv << std::setprecision(10);

  for (auto const &x : zone_volume)
    fzv << x << '\n';

  fzv.close();

  // Accumulate the zone-centered gradient
  zone_gradient.assign(mesh.zones.size(), VEC3_T(0.0));
#ifdef USE_SCORIA
  VEC3V_T packed_zg(num_local_corners);
  packed_zg.assign(num_local_corners, VEC3_T(0.0));

  VEC3V_T packed_pg(num_local_corners);
  packed_pg.assign(num_local_corners, VEC3_T(0.0));

  struct request req2;
  scoria_read(client, point_gradient.data(), num_local_corners,
      packed_pg.data(), mc_to_p_map.data(), NULL, 0, NONE, &req2);
  wait_request(client, &req2);

  std::fill(packed_zv.begin(), packed_zv.end(), 0);
  struct request req3;
  scoria_read(client, zone_volume.data(), num_local_corners, packed_zv.data(),
      mc_to_z_map.data(), NULL, 0, NONE, &req3);
  wait_request(client, &req3);

  for (int corner_idx = 0; corner_idx < num_local_corners; ++corner_idx) {
    if (corner_type[corner_idx] < 1)
      continue; // Only operate on interior corners
    packed_zg[corner_idx] = packed_pg[corner_idx] +
        corner_volume[corner_idx] / packed_zv[corner_idx];
  }

  struct request req4;
  scoria_write(client, zone_gradient.data(), num_local_corners,
      packed_zg.data(), mc_to_z_map.data(), NULL, 0, NONE, &req4);
  wait_request(client, &req4);
#else
  for (int corner_idx = 0; corner_idx < num_local_corners; ++corner_idx) {
    if (corner_type[corner_idx] < 1)
      continue; // Only operate on interior corners
    int const zone_idx = c_to_z_map[corner_idx];
    int const point_idx = c_to_p_map[corner_idx];
    double const c_z_vol_ratio =
        corner_volume[corner_idx] / zone_volume[zone_idx];
    zone_gradient[zone_idx] += point_gradient[point_idx] * c_z_vol_ratio;
  }
#endif

  std::ofstream fzg("zone_gradient.txt");
  fzg << std::setprecision(10);

  for (auto const &x : zone_gradient)
    fzg << x << '\n';

  fzg.close();

  mesh.zones.scatter(zone_gradient);
}

} // namespace Ume
