/*
  Copyright (c) 2023, Triad National Security, LLC. All rights reserved.

  This is open source software; you can redistribute it and/or modify it under
  the terms of the BSD-3 License. If software is modified to produce derivative
  works, such modified software should be clearly marked, so as not to confuse
  it with the version available from LANL. Full text of the BSD-3 License can be
  found in the LICENSE.md file, and the full assertion of copyright in the
  NOTICE.md file.
*/

/*!
  \file Ume/gradient.cc
*/

/*
 ** Scoria Includes
 */
#if defined(USE_SCORIA) && !defined(USE_CLIENT)
extern "C" {
#include "kernels.h"
}
#endif

/*
** Ume Includes
*/
#include "Ume/gradient.hh"

namespace Ume {

using DBLV_T = DS_Types::DBLV_T;
using VEC3V_T = DS_Types::VEC3V_T;
using VEC3_T = DS_Types::VEC3_T;

#if defined(USE_SCORIA) && defined(USE_CLIENT)
void gradzatp(struct client *client, Ume::SOA_Idx::Mesh &mesh,
    DBLV_T const &zone_field, VEC3V_T &point_gradient) {
#else
void gradzatp(Ume::SOA_Idx::Mesh &mesh, DBLV_T const &zone_field,
    VEC3V_T &point_gradient) {
#endif /* USE_SCORIA && USE_CLIENT */

  auto const &csurf = mesh.ds->caccess_vec3v("corner_csurf");
  auto const &corner_volume = mesh.ds->caccess_dblv("corner_vol");
  auto const &point_normal = mesh.ds->caccess_vec3v("point_norm");
  auto const &c_to_p_map = mesh.ds->caccess_intv("m:c>p");
  auto const &c_to_z_map = mesh.ds->caccess_intv("m:c>z");
  auto const &corner_type = mesh.corners.mask;
  auto const &point_type = mesh.points.mask;

#ifdef USE_SCORIA
#ifdef UME_PROFILING
  Ume::Timer gradzatp_scoria_map_time;
  gradzatp_scoria_map_time.start();
#endif /* UME_PROFILING */

  const UmeVector<size_t> mc_to_z_map(begin(c_to_z_map), end(c_to_z_map));
  const UmeVector<size_t> mc_to_p_map(begin(c_to_p_map), end(c_to_p_map));

#ifdef UME_PROFILING
  gradzatp_scoria_map_time.stop();
  mesh.levels.push_back(3);
  mesh.timelabels.push_back("gradzatp_scoria_map");
  mesh.times.push_back(gradzatp_scoria_map_time.seconds());
#endif /* UME_PROFILING */
#endif /* USE_SCORIA */

  int const pll = mesh.points.size();
  int const pl = mesh.points.local_size();
  int const cl = mesh.corners.local_size();

  DBLV_T point_volume(pll, 0.0);
  point_gradient.assign(pll, VEC3_T(0.0));

#ifdef USE_SCORIA
#ifdef UME_PROFILING
  Ume::Timer gradzatp_scoria_decompose_time;
  gradzatp_scoria_decompose_time.start();
#endif /* UME_PROFILING */

  const int num_threads = 22;

  DBLV_T packed_zf(cl, 0.0);

  DBLV_T packed_pv(cl, 0.0);

  DBLV_T packed_pg_x(cl, 0.0);
  DBLV_T packed_pg_y(cl, 0.0);
  DBLV_T packed_pg_z(cl, 0.0);

  DBLV_T point_gradient_x(point_gradient.size(), 0.0);
  DBLV_T point_gradient_y(point_gradient.size(), 0.0);
  DBLV_T point_gradient_z(point_gradient.size(), 0.0);

  for (size_t i = 0; i < point_gradient.size(); ++i) {
    point_gradient_x[i] = point_gradient[i][0];
    point_gradient_y[i] = point_gradient[i][1];
    point_gradient_z[i] = point_gradient[i][2];
  }

#ifdef UME_PROFILING
  gradzatp_scoria_decompose_time.stop();
  mesh.levels.push_back(3);
  mesh.timelabels.push_back("gradzatp_scoria_decompose");
  mesh.times.push_back(gradzatp_scoria_decompose_time.seconds());

  /* --------------------------- SCORIA ---------------------------- */
  Ume::Timer gradzatp_scoria_read_time;
  gradzatp_scoria_read_time.start();
#endif /* UME_PROFILING */

#ifdef USE_CLIENT
  struct request req1;
  scoria_read(client, zone_field.data(), cl, packed_zf.data(),
      mc_to_z_map.data(), NULL, num_threads, NONE, &req1);
  scoria_wait_request(client, &req1);
#else
  read_multi_thread_1(packed_zf.data(), zone_field.data(), cl,
      mc_to_z_map.data(), num_threads, NONE);
#endif /* USE_CLIENT */

#ifdef UME_PROFILING
  gradzatp_scoria_read_time.stop();
  mesh.levels.push_back(3);
  mesh.timelabels.push_back("gradzatp_scoria_read");
  mesh.times.push_back(gradzatp_scoria_read_time.seconds());

  Ume::Timer gradzatp_scoria_loop_time;
  gradzatp_scoria_loop_time.start();
#endif /* UME_PROFILING */

  for (int c = 0; c < cl; ++c) {
    if (corner_type[c] < 1)
      continue; // Only operate on interior corners
    packed_pv[c] = corner_volume[c];

    packed_pg_x[c] = csurf[c][0] * packed_zf[c];
    packed_pg_y[c] = csurf[c][1] * packed_zf[c];
    packed_pg_z[c] = csurf[c][2] * packed_zf[c];
  }

#ifdef UME_PROFILING
  gradzatp_scoria_loop_time.stop();
  mesh.levels.push_back(3);
  mesh.timelabels.push_back("gradzatp_scoria_loop");
  mesh.times.push_back(gradzatp_scoria_loop_time.seconds());

  Ume::Timer gradzatp_scoria_write_time;
  gradzatp_scoria_write_time.start();
#endif /*UME_PROFILING */

#ifdef USE_CLIENT
  struct request req2;
  scoria_writeadd(client, point_volume.data(), cl, packed_pv.data(),
      mc_to_p_map.data(), NULL, 0, NONE, &req2);
  scoria_wait_request(client, &req2);

  struct request req3;
  scoria_writeadd(client, point_gradient_x.data(), cl, packed_pg_x.data(),
      mc_to_p_map.data(), NULL, 0, NONE, &req3);
  scoria_wait_request(client, &req3);

  struct request req4;
  scoria_writeadd(client, point_gradient_y.data(), cl, packed_pg_y.data(),
      mc_to_p_map.data(), NULL, 0, NONE, &req4);
  scoria_wait_request(client, &req4);

  struct request req5;
  scoria_writeadd(client, point_gradient_z.data(), cl, packed_pg_z.data(),
      mc_to_p_map.data(), NULL, 0, NONE, &req5);
  scoria_wait_request(client, &req5);
#else
  writeadd_single_thread_1(
      point_volume.data(), packed_pv.data(), cl, mc_to_p_map.data(), NONE);
  writeadd_single_thread_1(point_gradient_x.data(), packed_pg_x.data(), cl,
      mc_to_p_map.data(), NONE);
  writeadd_single_thread_1(point_gradient_y.data(), packed_pg_y.data(), cl,
      mc_to_p_map.data(), NONE);
  writeadd_single_thread_1(point_gradient_z.data(), packed_pg_z.data(), cl,
      mc_to_p_map.data(), NONE);
#endif /* USE_CLIENT */

#ifdef UME_PROFILING
  gradzatp_scoria_write_time.stop();
  mesh.levels.push_back(3);
  mesh.timelabels.push_back("gradzatp_scoria_write");
  mesh.times.push_back(gradzatp_scoria_write_time.seconds());

  /* --------------------------- SCORIA ---------------------------- */

  Ume::Timer gradzatp_scoria_pack_time;
  gradzatp_scoria_pack_time.start();
#endif /* UME_PROFILING */

  for (size_t i = 0; i < point_gradient.size(); ++i) {
    point_gradient[i][0] = point_gradient_x[i];
    point_gradient[i][1] = point_gradient_y[i];
    point_gradient[i][2] = point_gradient_z[i];
  }

#ifdef UME_PROFILING
  gradzatp_scoria_pack_time.stop();
  mesh.levels.push_back(3);
  mesh.timelabels.push_back("gradzatp_scoria_pack");
  mesh.times.push_back(gradzatp_scoria_pack_time.seconds());
#endif /* UME_PROFILING */
#else
#ifdef UME_PROFILING
  Ume::Timer gradzatp_loop_time;
  gradzatp_loop_time.start();
#endif /* UME_PROFILING */

  for (int c = 0; c < cl; ++c) {
    if (corner_type[c] < 1)
      continue; // Only operate on interior corners
    int const z = c_to_z_map[c];
    int const p = c_to_p_map[c];
    point_volume[p] += corner_volume[c];
    point_gradient[p] += csurf[c] * zone_field[z];
  }

#ifdef UME_PROFILING
  gradzatp_loop_time.stop();
  mesh.levels.push_back(3);
  mesh.timelabels.push_back("gradzatp_loop");
  mesh.times.push_back(gradzatp_loop_time.seconds());
#endif /* UME_PROFILING */
#endif /* USE_SCORIA */

#ifdef UME_PROFILING
  Ume::Timer gradzatp_gathscat_time;
  gradzatp_gathscat_time.start();
#endif /* UME_PROFILING */

  mesh.points.gathscat(Ume::Comm::Op::SUM, point_volume);
  mesh.points.gathscat(Ume::Comm::Op::SUM, point_gradient);

#ifdef UME_PROFILING
  gradzatp_gathscat_time.stop();
  mesh.levels.push_back(3);
  mesh.timelabels.push_back("gradzatp_gathscat");
  mesh.times.push_back(gradzatp_gathscat_time.seconds());
#endif /* UME_PROFILING */

  /*
    Divide by point control volume to get gradient.  If a point is on the outer
    perimeter of the mesh (POINT_TYPE=-1), subtract the outward normal component
    of the gradient using the point normals.
   */
#ifdef UME_PROFILING
  Ume::Timer gradzatp_loop2_time;
  gradzatp_loop2_time.start();
#endif /* UME_PROFILING */

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
  
#ifdef UME_PROFILING
  gradzatp_loop2_time.stop();
  mesh.levels.push_back(3);
  mesh.timelabels.push_back("gradzatp_loop2");
  mesh.times.push_back(gradzatp_loop2_time.seconds());

  Ume::Timer gradzatp_scatter_time;
  gradzatp_scatter_time.start();
#endif /* UME_PROFILING */

  mesh.points.scatter(point_gradient);

#ifdef UME_PROFILING
  gradzatp_scatter_time.stop();
  mesh.levels.push_back(3);
  mesh.timelabels.push_back("gradzatp_scatter");
  mesh.times.push_back(gradzatp_scatter_time.seconds());
#endif /* UME_PROFILING */
}

#if defined(USE_SCORIA) && defined(USE_CLIENT)
void gradzatz(struct client *client, Ume::SOA_Idx::Mesh &mesh,
    DBLV_T const &zone_field, VEC3V_T &zone_gradient, VEC3V_T &point_gradient) {
#else
void gradzatz(Ume::SOA_Idx::Mesh &mesh, DBLV_T const &zone_field,
    VEC3V_T &zone_gradient, VEC3V_T &point_gradient) {
#endif

  auto const &c_to_z_map = mesh.ds->caccess_intv("m:c>z");
  auto const &c_to_p_map = mesh.ds->caccess_intv("m:c>p");
  int const num_local_corners = mesh.corners.local_size();
  auto const &corner_type = mesh.corners.mask;
  auto const &corner_volume = mesh.ds->caccess_dblv("corner_vol");

#ifdef USE_SCORIA
#ifdef UME_PROFILING
  Ume::Timer gradzatz_scoria_map_time;
  gradzatz_scoria_map_time.start();
#endif /* UME_PROFILING */

  const UmeVector<size_t> mc_to_z_map(begin(c_to_z_map), end(c_to_z_map));
  const UmeVector<size_t> mc_to_p_map(begin(c_to_p_map), end(c_to_p_map));

#ifdef UME_PROFILING
  gradzatz_scoria_map_time.stop();
  mesh.levels.push_back(2);
  mesh.timelabels.push_back("gradzatz_scoria_map");
  mesh.times.push_back(gradzatz_scoria_map_time.seconds());
#endif /* UME_PROFILING */
#endif

  // Get the field gradient at each mesh point.
#ifdef UME_PROFILING
  Ume::Timer gradzatp_time;
  gradzatp_time.start();
#endif /* UME_PROFILING */
#if defined(USE_SCORIA) && defined(USE_CLIENT)
  gradzatp(client, mesh, zone_field, point_gradient);
#else
  gradzatp(mesh, zone_field, point_gradient);
#endif
#ifdef UME_PROFILING
  gradzatp_time.stop();
  mesh.levels.push_back(2);
  mesh.timelabels.push_back("gradzatp");
  mesh.times.push_back(gradzatp_time.seconds());
#endif /* UME_PROFILING */

  /* Accumulate the zone volume.  Note that we need to allocate a zone field for
     volume, as we are accumulating from corners */
  DBLV_T zone_volume(mesh.zones.size(), 0.0);
#ifdef USE_SCORIA
#ifdef UME_PROFILING
  Ume::Timer gradzatz_scoria_loop1_time;
  gradzatz_scoria_loop1_time.start();
#endif /* UME_PROFILING */

  DBLV_T packed_zv(num_local_corners, 0.0);

  for (int corner_idx = 0; corner_idx < num_local_corners; ++corner_idx) {
    if (corner_type[corner_idx] < 1)
      continue; // Only operate on interior corners
    packed_zv[corner_idx] = corner_volume[corner_idx];
  }
#ifdef UME_PROFILING
  gradzatz_scoria_loop1_time.stop();
  mesh.levels.push_back(2);
  mesh.timelabels.push_back("gradzatz_scoria_loop1");
  mesh.times.push_back(gradzatz_scoria_loop1_time.seconds());
  
  Ume::Timer gradzatz_scoria_write1_time;
  gradzatz_scoria_write1_time.start();
#endif /* UME_PROFILING */
#ifdef USE_CLIENT
  struct request req1;
  scoria_writeadd(client, zone_volume.data(), num_local_corners,
      packed_zv.data(), mc_to_z_map.data(), NULL, 0, NONE, &req1);
  scoria_wait_request(client, &req1);
#else
  writeadd_single_thread_1(zone_volume.data(), packed_zv.data(),
      num_local_corners, mc_to_z_map.data(), NONE);
#endif
#ifdef UME_PROFILING
  gradzatz_scoria_write1_time.stop();
  mesh.levels.push_back(2);
  mesh.timelabels.push_back("gradzatz_scoria_write1");
  mesh.times.push_back(gradzatz_scoria_write1_time.seconds());
#endif /* UME_PROFILING */

#else
#ifdef UME_PROFILING
  Ume::Timer gradzatz_loop1_time;
  gradzatz_loop1_time.start();
#endif /* UME_PROFILING */

  for (int corner_idx = 0; corner_idx < num_local_corners; ++corner_idx) {
    if (corner_type[corner_idx] < 1)
      continue; // Only operate on interior corners
    int const zone_idx = c_to_z_map[corner_idx];
    /* Note that we cannot parallelize across corners, as multiple corners
       write to the same zone. */
    zone_volume[zone_idx] += corner_volume[corner_idx];
  }

#ifdef UME_PROFILING
  gradzatz_loop1_time.stop();
  mesh.levels.push_back(2);
  mesh.timelabels.push_back("gradzatz_loop1");
  mesh.times.push_back(gradzatz_loop1_time.seconds());
#endif /* UME_PROFILING */
#endif

  // Accumulate the zone-centered gradient
  zone_gradient.assign(mesh.zones.size(), VEC3_T(0.0));
#ifdef USE_SCORIA
#ifdef UME_PROFILING
  Ume::Timer gradzatz_scoria_decompose_time;
  gradzatz_scoria_decompose_time.start();
#endif /* UME_PROFILING */

  const int num_threads = 22;

  DBLV_T packed_pg_x(num_local_corners, 0.0);
  DBLV_T packed_pg_y(num_local_corners, 0.0);
  DBLV_T packed_pg_z(num_local_corners, 0.0);

  DBLV_T point_gradient_x(point_gradient.size(), 0.0);
  DBLV_T point_gradient_y(point_gradient.size(), 0.0);
  DBLV_T point_gradient_z(point_gradient.size(), 0.0);

  for (size_t i = 0; i < point_gradient.size(); ++i) {
    point_gradient_x[i] = point_gradient[i][0];
    point_gradient_y[i] = point_gradient[i][1];
    point_gradient_z[i] = point_gradient[i][2];
  }

  DBLV_T packed_zg_x(num_local_corners, 0.0);
  DBLV_T packed_zg_y(num_local_corners, 0.0);
  DBLV_T packed_zg_z(num_local_corners, 0.0);

  DBLV_T zone_gradient_x(zone_gradient.size(), 0.0);
  DBLV_T zone_gradient_y(zone_gradient.size(), 0.0);
  DBLV_T zone_gradient_z(zone_gradient.size(), 0.0);

  std::fill(packed_zv.begin(), packed_zv.end(), 0);

#ifdef UME_PROFILING
  gradzatz_scoria_decompose_time.stop();
  mesh.levels.push_back(2);
  mesh.timelabels.push_back("gradzatz_scoria_decompose");
  mesh.times.push_back(gradzatz_scoria_decompose_time.seconds());
  /* --------------------------- SCORIA ---------------------------- */
  Ume::Timer gradzatz_scoria_read2_time;
  gradzatz_scoria_read2_time.start();
#endif /* UME_PROFILING */

#ifdef USE_CLIENT
  struct request req2;
  scoria_read(client, point_gradient_x.data(), num_local_corners,
      packed_pg_x.data(), mc_to_p_map.data(), NULL, num_threads, NONE, &req2);
  scoria_wait_request(client, &req2);

  struct request req3;
  scoria_read(client, point_gradient_y.data(), num_local_corners,
      packed_pg_y.data(), mc_to_p_map.data(), NULL, num_threads, NONE, &req3);
  scoria_wait_request(client, &req3);

  struct request req4;
  scoria_read(client, point_gradient_z.data(), num_local_corners,
      packed_pg_z.data(), mc_to_p_map.data(), NULL, num_threads, NONE, &req4);
  scoria_wait_request(client, &req4);

  struct request req5;
  scoria_read(client, zone_volume.data(), num_local_corners, packed_zv.data(),
      mc_to_z_map.data(), NULL, num_threads, NONE, &req5);
  scoria_wait_request(client, &req5);
#else
  read_multi_thread_1(packed_pg_x.data(), point_gradient_x.data(),
      num_local_corners, mc_to_p_map.data(), num_threads, NONE);
  read_multi_thread_1(packed_pg_y.data(), point_gradient_y.data(),
      num_local_corners, mc_to_p_map.data(), num_threads, NONE);
  read_multi_thread_1(packed_pg_z.data(), point_gradient_z.data(),
      num_local_corners, mc_to_p_map.data(), num_threads, NONE);
  read_multi_thread_1(packed_zv.data(), zone_volume.data(), num_local_corners,
      mc_to_z_map.data(), num_threads, NONE);
#endif
#ifdef UME_PROFILING
  gradzatz_scoria_read2_time.stop();
  mesh.levels.push_back(2);
  mesh.timelabels.push_back("gradzatz_scoria_read2");
  mesh.times.push_back(gradzatz_scoria_read2_time.seconds());

  Ume::Timer gradzatz_scoria_loop2_time;
  gradzatz_scoria_loop2_time.start();
#endif /* UME_PROFILING */

  for (int corner_idx = 0; corner_idx < num_local_corners; ++corner_idx) {
    if (corner_type[corner_idx] < 1)
      continue; // Only operate on interior corners
    packed_zg_x[corner_idx] = packed_pg_x[corner_idx] *
        corner_volume[corner_idx] / packed_zv[corner_idx];
    packed_zg_y[corner_idx] = packed_pg_y[corner_idx] *
        corner_volume[corner_idx] / packed_zv[corner_idx];
    packed_zg_z[corner_idx] = packed_pg_z[corner_idx] *
        corner_volume[corner_idx] / packed_zv[corner_idx];
  }

#ifdef UME_PROFILING
  gradzatz_scoria_loop2_time.stop();
  mesh.levels.push_back(2);
  mesh.timelabels.push_back("gradzatz_scoria_loop2");
  mesh.times.push_back(gradzatz_scoria_loop2_time.seconds());

  Ume::Timer gradzatz_scoria_write2_time;
  gradzatz_scoria_write2_time.start();
#endif /* UME_PROFILING */

#ifdef USE_CLIENT
  struct request req6;
  scoria_writeadd(client, zone_gradient_x.data(), num_local_corners,
      packed_zg_x.data(), mc_to_z_map.data(), NULL, 0, NONE, &req6);
  scoria_wait_request(client, &req6);

  struct request req7;
  scoria_writeadd(client, zone_gradient_y.data(), num_local_corners,
      packed_zg_y.data(), mc_to_z_map.data(), NULL, 0, NONE, &req7);
  scoria_wait_request(client, &req7);

  struct request req8;
  scoria_writeadd(client, zone_gradient_z.data(), num_local_corners,
      packed_zg_z.data(), mc_to_z_map.data(), NULL, 0, NONE, &req8);
  scoria_wait_request(client, &req8);
#else
  writeadd_single_thread_1(zone_gradient_x.data(), packed_zg_x.data(),
      num_local_corners, mc_to_z_map.data(), NONE);
  writeadd_single_thread_1(zone_gradient_y.data(), packed_zg_y.data(),
      num_local_corners, mc_to_z_map.data(), NONE);
  writeadd_single_thread_1(zone_gradient_z.data(), packed_zg_z.data(),
      num_local_corners, mc_to_z_map.data(), NONE);
#endif
#ifdef UME_PROFILING
  gradzatz_scoria_write2_time.stop();
  mesh.levels.push_back(2);
  mesh.timelabels.push_back("gradzatz_scoria_write2");
  mesh.times.push_back(gradzatz_scoria_write2_time.seconds());
  /* --------------------------- SCORIA ---------------------------- */

  Ume::Timer gradzatz_scoria_pack_time;
  gradzatz_scoria_pack_time.start();  
#endif /* UME_PROFILING */

  for (size_t i = 0; i < zone_gradient.size(); ++i) {
    zone_gradient[i][0] = zone_gradient_x[i];
    zone_gradient[i][1] = zone_gradient_y[i];
    zone_gradient[i][2] = zone_gradient_z[i];
  }

#ifdef UME_PROFILING
  gradzatz_scoria_pack_time.stop();
  mesh.levels.push_back(2);
  mesh.timelabels.push_back("gradzatz_scoria_pack");
  mesh.times.push_back(gradzatz_scoria_pack_time.seconds());
#endif /* UME_PROFILING */

#else
#ifdef UME_PROFILING
  Ume::Timer gradzatz_loop2_time;
  gradzatz_loop2_time.start();
#endif /* UME_PROFILING */
  for (int corner_idx = 0; corner_idx < num_local_corners; ++corner_idx) {
    if (corner_type[corner_idx] < 1)
      continue; // Only operate on interior corners
    int const zone_idx = c_to_z_map[corner_idx];
    int const point_idx = c_to_p_map[corner_idx];
    double const c_z_vol_ratio =
        corner_volume[corner_idx] / zone_volume[zone_idx];
    zone_gradient[zone_idx] += point_gradient[point_idx] * c_z_vol_ratio;
  }

#ifdef UME_PROFILING
  gradzatz_loop2_time.stop();
  mesh.levels.push_back(2);
  mesh.timelabels.push_back("gradzatz_loop2");
  mesh.times.push_back(gradzatz_loop2_time.seconds());
#endif /* UME_PROFILING */
#endif
#ifdef UME_PROFILING
  Ume::Timer gradzatz_scatter_time;
  gradzatz_scatter_time.start();
#endif /* UME_PROFILING */
  mesh.zones.scatter(zone_gradient);
#ifdef UME_PROFILING
  gradzatz_scatter_time.stop();
  mesh.levels.push_back(2);
  mesh.timelabels.push_back("gradzatz_scatter");
  mesh.times.push_back(gradzatz_scatter_time.seconds());
#endif /* UME_PROFILING */
}

#if defined(USE_SCORIA) && defined(USE_CLIENT)
void gradzatp_invert(struct client *client, Ume::SOA_Idx::Mesh &mesh,
    DBLV_T const &zone_field, VEC3V_T &point_gradient) {
#else
void gradzatp_invert(Ume::SOA_Idx::Mesh &mesh, DBLV_T const &zone_field,
    VEC3V_T &point_gradient) {
#endif
  auto const &csurf = mesh.ds->caccess_vec3v("corner_csurf");
  auto const &corner_volume = mesh.ds->caccess_dblv("corner_vol");
  auto const &point_normal = mesh.ds->caccess_vec3v("point_norm");
  auto const &p_to_c_map = mesh.ds->caccess_intrr("m:p>rc");
  auto const &c_to_z_map = mesh.ds->caccess_intv("m:c>z");
  auto const &point_type = mesh.points.mask;

#ifdef USE_SCORIA
//  const UmeVector<size_t> mp_to_c_map(begin(p_to_c_map), end(p_to_c_map));
  const UmeVector<size_t> mc_to_z_map(begin(c_to_z_map), end(c_to_z_map));
#endif

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

#if defined(USE_SCORIA) && defined(USE_CLIENT)
void gradzatz_invert(struct client *client, Ume::SOA_Idx::Mesh &mesh,
    DBLV_T const &zone_field, VEC3V_T &zone_gradient, VEC3V_T &point_gradient) {
#else
void gradzatz_invert(Ume::SOA_Idx::Mesh &mesh, DBLV_T const &zone_field,
    VEC3V_T &zone_gradient, VEC3V_T &point_gradient) {
#endif
  auto const &z_to_c_map = mesh.ds->caccess_intrr("m:z>c");
  auto const &c_to_p_map = mesh.ds->caccess_intv("m:c>p");
  int const num_local_zones = mesh.zones.local_size();
  auto const &zone_type = mesh.zones.mask;
  auto const &corner_volume = mesh.ds->caccess_dblv("corner_vol");

#ifdef USE_SCORIA
  //const UmeVector<size_t> mz_to_c_map(begin(z_to_c_map), end(z_to_c_map));
  const UmeVector<size_t> mc_to_p_map(begin(c_to_p_map), end(c_to_p_map));
#endif

  // Get the field gradient at each mesh point.
#if defined(USE_SCORIA) && defined(USE_CLIENT)
  gradzatp_invert(client, mesh, zone_field, point_gradient);
#else
  gradzatp_invert(mesh, zone_field, point_gradient);
#endif

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

