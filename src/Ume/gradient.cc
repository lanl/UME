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

#ifdef USE_CALI
#include <caliper/cali-manager.h>
#include <caliper/cali.h>
#endif

/*
** Ume Includes
*/
#include "Ume/gradient.hh"
#include <stdlib.h>

#define TILE_SIZE 128
namespace Ume {

using DBLV_T = DS_Types::DBLV_T; // vector double
using VEC3V_T = DS_Types::VEC3V_T;
using VEC3_T = DS_Types::VEC3_T;

#ifdef USE_SCORIA
#define NTHREADS 22 // specific hardware perform the gather/scatter
#endif

#if defined(USE_SCORIA) && defined(USE_CLIENT)
void gradzatp(struct client *client, Ume::SOA_Idx::Mesh &mesh,
    DBLV_T const &zone_field, VEC3V_T &point_gradient) {
#else
void gradzatp(Ume::SOA_Idx::Mesh &mesh, DBLV_T const &zone_field,
    VEC3V_T &point_gradient) {
#endif

#ifdef USE_CALI
  CALI_CXX_MARK_FUNCTION;
#endif

  auto const &csurf = mesh.ds->caccess_vec3v("corner_csurf");
  auto const &corner_volume = mesh.ds->caccess_dblv("corner_vol");
  auto const &point_normal = mesh.ds->caccess_vec3v("point_norm");
  auto const &c_to_p_map = mesh.ds->caccess_intv("m:c>p");
  auto const &c_to_z_map = mesh.ds->caccess_intv("m:c>z");
  auto const &corner_type = mesh.corners.mask;
  auto const &point_type = mesh.points.mask;

#ifdef USE_SCORIA
#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatp_Scoria_Map");
#endif
  const int num_threads = NTHREADS;

  const UmeVector<size_t> mc_to_z_map(
      begin(c_to_z_map), end(c_to_z_map)); // std::vector with shm_allocator
  const UmeVector<size_t> mc_to_p_map(begin(c_to_p_map), end(c_to_p_map));
#ifdef USE_CALI
  CALI_MARK_END("Gradzatp_Scoria_Map");
#endif
#endif

  int const pll = mesh.points.size();
  int const pl = mesh.points.local_size();
  int const cl = mesh.corners.local_size();

#ifdef USE_SCORIA
#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatp_Point_Volume_Scoria_Pack");
#endif
  DBLV_T packed_zf(cl, 0.0);
#ifdef USE_CALI
  CALI_MARK_END("Gradzatp_Point_Volume_Scoria_Pack");
#endif

  /* --------------------------- SCORIA ---------------------------- */
#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatp_Point_Volume_Scoria_Read");
#endif
#ifdef USE_CLIENT
  struct request req1;
  scoria_read(client, zone_field.data(), cl, packed_zf.data(),
      mc_to_z_map.data(), NULL, num_threads, NONE, &req1);
  scoria_wait_request(client, &req1);
#else
  read_multi_thread_1(packed_zf.data(), zone_field.data(), cl,
      mc_to_z_map.data(), num_threads, NONE);
#endif
#ifdef USE_CALI
  CALI_MARK_END("Gradzatp_Point_Volume_Scoria_Read");
#endif

#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatp_Point_Volume_Scoria_Pack");
#endif
  DBLV_T packed_pv(cl, 0.0);

  DBLV_T packed_pg_x(cl, 0.0);
  DBLV_T packed_pg_y(cl, 0.0);
  DBLV_T packed_pg_z(cl, 0.0);
#ifdef USE_CALI
  CALI_MARK_END("Gradzatp_Point_Volume_Scoria_Pack");
#endif

// Used in Scoria transform code
#ifdef USE_CALI
  CALI_CXX_MARK_LOOP_BEGIN(
      gradzatp_point_volume_scoria_loop_id, "Gradzatp_Point_Volume_Loop_2D");
#endif
  // std::cout<<"cl is: "<<cl<<std::endl; 2D loop, need to check it.
for(int c=0;c<cl;c+=TILE_SIZE)
 {
   for(int j=c;j<std::min(TILE_SIZE+c,cl);j++)
   {
    if (corner_type[j] < 1)
      continue; // Only operate on interior corners
    packed_pv[j] = corner_volume[j];

    packed_pg_x[j] = csurf[j][0] * packed_zf[j];
    packed_pg_y[j] = csurf[j][1] * packed_zf[j];
    packed_pg_z[j] = csurf[j][2] * packed_zf[j];
   }
 }
  
/*for (int c = 0; c < cl; ++c) {
    if (corner_type[c] < 1)
      continue; // Only operate on interior corners
    packed_pv[c] = corner_volume[c];

    packed_pg_x[c] = csurf[c][0] * packed_zf[c];
    packed_pg_y[c] = csurf[c][1] * packed_zf[c];
    packed_pg_z[c] = csurf[c][2] * packed_zf[c];
  }*/
  
#ifdef USE_CALI
  CALI_CXX_MARK_LOOP_END(gradzatp_point_volume_scoria_loop_id);
#endif

#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatp_Point_Volume_Scoria_Pack");
#endif
  DBLV_T point_volume(pll, 0.0);
  point_gradient.assign(pll, VEC3_T(0.0));

  DBLV_T point_gradient_x(point_gradient.size(), 0.0);
  DBLV_T point_gradient_y(point_gradient.size(), 0.0);
  DBLV_T point_gradient_z(point_gradient.size(), 0.0);
#ifdef USE_CALI
  CALI_MARK_END("Gradzatp_Point_Volume_Scoria_Pack");
#endif

#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatp_Point_Volume_Scoria_Write");
#endif
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
#endif
#ifdef USE_CALI
  CALI_MARK_END("Gradzatp_Point_Volume_Scoria_Write");
#endif
  /* --------------------------- SCORIA ---------------------------- */

#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatp_Point_Volume_Scoria_Unpack");
#endif
  // std::cout<<"Point gradient is: "<<point_gradient.size()<<std::endl; 1D to
  // 2D
 for (size_t i = 0; i < point_gradient.size(); i += TILE_SIZE) {
    for (size_t j = i; j < std::min(TILE_SIZE+i,point_gradient.size()); j++) {
    point_gradient[j][0] = point_gradient_x[j];
    point_gradient[j][1] = point_gradient_y[j];
    point_gradient[j][2] = point_gradient_z[j];
    }
  }

  /*for (size_t i = 0; i < point_gradient.size(); ++i) {
    point_gradient[i][0] = point_gradient_x[i];
    point_gradient[i][1] = point_gradient_y[i];
    point_gradient[i][2] = point_gradient_z[i];
  }*/
#ifdef USE_CALI
  CALI_MARK_END("Gradzatp_Point_Volume_Scoria_Unpack");
#endif
#else
  DBLV_T point_volume(pll, 0.0);
  point_gradient.assign(pll, VEC3_T(0.0));

#ifdef USE_CALI
  CALI_CXX_MARK_LOOP_BEGIN(
      gradzatp_point_volume_base_loop_id, "Gradzatp_Point_Volume_Loop_1D");
#endif
  // Not executed in this input deck
  for (int c = 0; c < cl; ++c) {
    if (corner_type[c] < 1)
      continue; // Only operate on interior corners
    int const z = c_to_z_map[c];
    int const p = c_to_p_map[c];
    point_volume[p] += corner_volume[c];
    point_gradient[p] += csurf[c] * zone_field[z];
  }

#ifdef USE_CALI
  CALI_CXX_MARK_LOOP_END(gradzatp_point_volume_base_loop_id);
#endif
#endif

#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatp_GathScat");
#endif
  mesh.points.gathscat(Ume::Comm::Op::SUM, point_volume);
  mesh.points.gathscat(Ume::Comm::Op::SUM, point_gradient);
#ifdef USE_CALI
  CALI_MARK_END("Gradzatp_GathScat");
#endif

  /*
    Divide by point control volume to get gradient.  If a point is on the outer
    perimeter of the mesh (POINT_TYPE=-1), subtract the outward normal component
    of the gradient using the point normals.
   */
#ifdef USE_CALI
  CALI_CXX_MARK_LOOP_BEGIN(
      gradzatp_point_gradient_base_loop_id, "Gradzatp_Point_Gradient_Loop");
#endif
  // std::cout<<"pl is "<<pl<<std::endl;

  for (int p = 0; p < pl; p += TILE_SIZE) {
    for (int j = p; j < std::min(p + TILE_SIZE, pl); ++j) {
      if (point_type[j] > 0) {
        // Internal point
        point_gradient[j] /= point_volume[j];
      } else if (point_type[j] == -1) {
        // Mesh boundary point
        double const ppdot = dotprod(point_gradient[j], point_normal[j]);
        point_gradient[j] =
            (point_gradient[j] - point_normal[j] * ppdot) / point_volume[j];
      }
    }
  }

  /*for (int p = 0; p < pl; ++p) {
    if (point_type[p] > 0) {
      // Internal point
      point_gradient[p] /= point_volume[p];
    } else if (point_type[p] == -1) {
      // Mesh boundary point
      double const ppdot = dotprod(point_gradient[p], point_normal[p]);
      point_gradient[p] =
          (point_gradient[p] - point_normal[p] * ppdot) / point_volume[p];
    }
  }*/
#ifdef USE_CALI
  CALI_CXX_MARK_LOOP_END(gradzatp_point_gradient_base_loop_id);
#endif

#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatp_Scatter");
#endif
  mesh.points.scatter(point_gradient);
#ifdef USE_CALI
  CALI_MARK_END("Gradzatp_Scatter");
#endif
}

#if defined(USE_SCORIA) && defined(USE_CLIENT)
void gradzatz(struct client *client, Ume::SOA_Idx::Mesh &mesh,
    DBLV_T const &zone_field, VEC3V_T &zone_gradient, VEC3V_T &point_gradient) {
#else
void gradzatz(Ume::SOA_Idx::Mesh &mesh, DBLV_T const &zone_field,
    VEC3V_T &zone_gradient, VEC3V_T &point_gradient) {
#endif

#ifdef USE_CALI
  CALI_CXX_MARK_FUNCTION;
#endif

  auto const &c_to_z_map = mesh.ds->caccess_intv("m:c>z");
  auto const &c_to_p_map = mesh.ds->caccess_intv("m:c>p");

#ifdef USE_SCORIA
#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatz_Scoria_Map");
#endif
  const int num_threads = NTHREADS;

  const UmeVector<size_t> mc_to_z_map(begin(c_to_z_map), end(c_to_z_map));
  const UmeVector<size_t> mc_to_p_map(begin(c_to_p_map), end(c_to_p_map));
#ifdef USE_CALI
  CALI_MARK_END("Gradzatz_Scoria_Map");
#endif
#endif

  // Get the field gradient at each mesh point.
#if defined(USE_SCORIA) && defined(USE_CLIENT)
  gradzatp(client, mesh, zone_field, point_gradient);
#else
  gradzatp(mesh, zone_field, point_gradient);
#endif

  int const num_local_corners = mesh.corners.local_size();
  auto const &corner_type = mesh.corners.mask;
  auto const &corner_volume = mesh.ds->access_dblv("corner_vol");

  /* Accumulate the zone volume.  Note that we need to allocate a zone field for
     volume, as we are accumulating from corners */
#ifdef USE_SCORIA
  DBLV_T packed_zv(num_local_corners, 0.0);
#ifdef USE_CALI
  CALI_CXX_MARK_LOOP_BEGIN(
      gradzatz_zone_volume_scoria_loop_id, "Gradzatz_Zone_Volume_Loop");
#endif
  // Potential change of loop
  // std::cout<<"local_corners is "<<num_local_corners<<std::endl;

  for (int corner_idx = 0; corner_idx < num_local_corners;
       corner_idx += TILE_SIZE) {
    for (int j = corner_idx;
         j < std::min(corner_idx + TILE_SIZE, num_local_corners); j++) {
      if (corner_type[j] < 1)
        continue; // Only operate on interior corners
      packed_zv[j] = corner_volume[j];
    }
  }

/*  for (int corner_idx = 0; corner_idx < num_local_corners; ++corner_idx) {
    if (corner_type[corner_idx] < 1)
      continue; // Only operate on interior corners
    packed_zv[corner_idx] = corner_volume[corner_idx];
  }*/
#ifdef USE_CALI
  CALI_CXX_MARK_LOOP_END(gradzatz_zone_volume_scoria_loop_id);
#endif

  DBLV_T zone_volume(mesh.zones.size(), 0.0);

#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatz_Zone_Volume_Scoria_Write");
#endif
#ifdef USE_CLIENT
  struct request req1;
  scoria_writeadd(client, zone_volume.data(), num_local_corners,
      packed_zv.data(), mc_to_z_map.data(), NULL, 0, NONE, &req1);
  scoria_wait_request(client, &req1);
#else
  writeadd_single_thread_1(zone_volume.data(), packed_zv.data(),
      num_local_corners, mc_to_z_map.data(), NONE);
#endif
#ifdef USE_CALI
  CALI_MARK_END("Gradzatz_Zone_Volume_Scoria_Write");
#endif

#else
  DBLV_T zone_volume(mesh.zones.size(), 0.0);
#ifdef USE_CALI
  CALI_CXX_MARK_LOOP_BEGIN(
      gradzatz_zone_volume_base_loop_id, "Gradzatz_Zone_Volume_Loop");
#endif
  // Changed the loop
  for (int corner_idx = 0; corner_idx < num_local_corners;
       corner_idx += TILE_SIZE) {
    for (int j = corner_idx;
         j < std::min(corner_idx + TILE_SIZE, num_local_corners); j++) {
      if (corner_type[j] < 1)
        continue; // Only operate on interior corners
      int const zone_idx = c_to_z_map[j];
      zone_volume[zone_idx] += corner_volume[corner_idx];
    }
  }

  /*for (int corner_idx = 0; corner_idx < num_local_corners; ++corner_idx) {
    if (corner_type[corner_idx] < 1)
      continue; // Only operate on interior corners
    int const zone_idx = c_to_z_map[corner_idx];
    /* Note that we cannot parallelize across corners, as multiple corners
       write to the same zone. */
  /*zone_volume[zone_idx] += corner_volume[corner_idx];
}*/

#ifdef USE_CALI
  CALI_CXX_MARK_LOOP_END(gradzatz_zone_volume_base_loop_id);
#endif
#endif

  // Accumulate the zone-centered gradient
  zone_gradient.assign(mesh.zones.size(), VEC3_T(0.0));

#ifdef USE_SCORIA
#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatz_Zone_Gradient_Scoria_Pack");
#endif
  DBLV_T packed_pg_x(num_local_corners, 0.0);
  DBLV_T packed_pg_y(num_local_corners, 0.0);
  DBLV_T packed_pg_z(num_local_corners, 0.0);

  DBLV_T point_gradient_x(point_gradient.size(), 0.0);
  DBLV_T point_gradient_y(point_gradient.size(), 0.0);
  DBLV_T point_gradient_z(point_gradient.size(), 0.0);

  for (size_t i = 0; i < point_gradient.size(); i += TILE_SIZE) {
    for (size_t j = i; j < std::min(TILE_SIZE+i,point_gradient.size()); j++) {
    point_gradient_x[j] = point_gradient[j][0];
    point_gradient_y[j] = point_gradient[j][1];
    point_gradient_z[j] = point_gradient[j][2];
    }
  }

 /*for (size_t i = 0; i < point_gradient.size(); ++i) {
    point_gradient_x[i] = point_gradient[i][0];
    point_gradient_y[i] = point_gradient[i][1];
    point_gradient_z[i] = point_gradient[i][2];
  }*/

  std::fill(packed_zv.begin(), packed_zv.end(), 0);
#ifdef USE_CALI
  CALI_MARK_END("Gradzatz_Zone_Gradient_Scoria_Pack");
#endif
  /* --------------------------- SCORIA ---------------------------- */
#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatz_Zone_Gradient_Scoria_Read");
#endif
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
#ifdef USE_CALI
  CALI_MARK_END("Gradzatz_Zone_Gradient_Scoria_Read");
#endif

#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatz_Zone_Gradient_Scoria_Pack");
#endif
  corner_type = mesh.corners.mask;
  corner_volume = mesh.ds->caccess_dblv("corner_vol");

  DBLV_T packed_zg_x(num_local_corners, 0.0);
  DBLV_T packed_zg_y(num_local_corners, 0.0);
  DBLV_T packed_zg_z(num_local_corners, 0.0);
#ifdef USE_CALI
  CALI_MARK_END("Gradzatz_Zone_Gradient_Scoria_Pack");
#endif

#ifdef USE_CALI
  CALI_CXX_MARK_LOOP_BEGIN(
      gradzatz_zone_gradient_scoria_loop_id, "Gradzatz_Zone_Gradient_Loop");
#endif
  // Change the loop
  /* for (int corner_idx = 0; corner_idx < num_local_corners; ++corner_idx) {
     if (corner_type[corner_idx] < 1)
       continue; // Only operate on interior corners
     double const c_z_vol_ratio =
         corner_volume[corner_idx] / packed_zv[corner_idx];

     packed_zg_x[corner_idx] = packed_pg_x[corner_idx] * c_z_vol_ratio;
     packed_zg_y[corner_idx] = packed_pg_y[corner_idx] * c_z_vol_ratio;
     packed_zg_z[corner_idx] = packed_pg_z[corner_idx] * c_z_vol_ratio;
   }*/

  for (int corner_idx = 0; corner_idx < num_local_corners;
       corner_idx += TILE_SIZE) {
    for (int j = corner_idx;
         j < std::min(corner_idx + TILE_SIZE, num_local_corners); j++) {
      if (corner_type[j] < 1)
        continue; // Only operate on interior corners
      double const c_z_vol_ratio = corner_volume[j] / packed_zv[j];

      packed_zg_x[j] = packed_pg_x[j] * c_z_vol_ratio;
      packed_zg_y[j] = packed_pg_y[j] * c_z_vol_ratio;
      packed_zg_z[j] = packed_pg_z[j] * c_z_vol_ratio;
    }
  }

#ifdef USE_CALI
  CALI_CXX_MARK_LOOP_END(gradzatz_zone_gradient_scoria_loop_id);
#endif

#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatz_Zone_Gradient_Scoria_Pack");
#endif
  DBLV_T zone_gradient_x(zone_gradient.size(), 0.0);
  DBLV_T zone_gradient_y(zone_gradient.size(), 0.0);
  DBLV_T zone_gradient_z(zone_gradient.size(), 0.0);
#ifdef USE_CALI
  CALI_MARK_END("Gradzatz_Zone_Gradient_Scoria_Pack");
#endif

#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatz_Zone_Gradient_Scoria_Write");
#endif
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
#ifdef USE_CALI
  CALI_MARK_END("Gradzatz_Zone_Gradient_Scoria_Write");
#endif
  /* --------------------------- SCORIA ---------------------------- */

#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatz_Zone_Gradient_Scoria_Unpack");
#endif
  // 2D loop
  for (size_t i = 0; i < zone_gradient.size(); i += TILE_SIZE) {
    for (size_t j = i; j < std::min(TILE_SIZE+i,zone_gradient.size()); j++) {
      //int idx = i + j;
    zone_gradient[j][0] = zone_gradient_x[j];
    zone_gradient[j][1] = zone_gradient_y[j];
    zone_gradient[j][2] = zone_gradient_z[j];
    }
  }
  /*for (size_t i = 0; i < zone_gradient.size(); ++i) {
    zone_gradient[i][0] = zone_gradient_x[i];
    zone_gradient[i][1] = zone_gradient_y[i];
    zone_gradient[i][2] = zone_gradient_z[i];
  }*/
#ifdef USE_CALI
  CALI_MARK_END("Gradzatz_Zone_Gradient_Scoria_Unpack");
#endif

#else
#ifdef USE_CALI
  CALI_CXX_MARK_LOOP_BEGIN(
      gradzatz_zone_gradient_base_loop_id, "Gradzatz_Zone_Gradient_Loop");
#endif
  // Changed to tiling
  for (int corner_idx = 0; corner_idx < num_local_corners;
       corner_idx += TILE_SIZE) {
    for (int j = corner_idx;
         j < std::min(corner_idx + TILE_SIZE, num_local_corners); j++) {
      if (corner_type[j] < 1)
        continue; // Only operate on interior corners
      int const zone_idx = c_to_z_map[j];
      int const point_idx = c_to_p_map[j];
      double const c_z_vol_ratio = corner_volume[j] / zone_volume[zone_idx];
      zone_gradient[zone_idx] += point_gradient[point_idx] * c_z_vol_ratio;
    }
  }

  /*for (int corner_idx = 0; corner_idx < num_local_corners; ++corner_idx) {
    if (corner_type[corner_idx] < 1)
      continue; // Only operate on interior corners
    int const zone_idx = c_to_z_map[corner_idx];
    int const point_idx = c_to_p_map[corner_idx];
    double const c_z_vol_ratio =
        corner_volume[corner_idx] / zone_volume[zone_idx];
    zone_gradient[zone_idx] += point_gradient[point_idx] * c_z_vol_ratio;
  }*/
#ifdef USE_CALI
  CALI_CXX_MARK_LOOP_END(gradzatz_zone_gradient_base_loop_id);
#endif
#endif

#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatz_Scatter");
#endif
  mesh.zones.scatter(zone_gradient);
#ifdef USE_CALI
  CALI_MARK_END("Gradzatz_Scatter");
#endif
}

#if defined(USE_SCORIA) && defined(USE_CLIENT)
void gradzatp_invert(struct client *client, Ume::SOA_Idx::Mesh &mesh,
    DBLV_T const &zone_field, VEC3V_T &point_gradient) {
#else
void gradzatp_invert(Ume::SOA_Idx::Mesh &mesh, DBLV_T const &zone_field,
    VEC3V_T &point_gradient) {
#endif

#ifdef USE_CALI
  CALI_CXX_MARK_FUNCTION;
#endif

  auto const &csurf = mesh.ds->caccess_vec3v("corner_csurf");
  auto const &corner_volume = mesh.ds->caccess_dblv("corner_vol");
  auto const &point_normal = mesh.ds->caccess_vec3v("point_norm");
  auto const &p_to_c_map = mesh.ds->caccess_intrr("m:p>rc");
  auto const &c_to_z_map = mesh.ds->caccess_intv("m:c>z");
  auto const &point_type = mesh.points.mask;

  int const num_points = mesh.points.size();
  int const num_local_points = mesh.points.local_size();

#ifdef USE_SCORIA
#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatp_Invert_Scoria_Map");
#endif
  const int num_threads = NTHREADS;

  UmeVector<size_t> mp_to_c_map;
  UmeVector<size_t> mp_to_c_count(num_local_points + 1);

  int corner_count = 0;
  mp_to_c_count[0] = 0;
  // Invert part of the code, need to check something like this in the trail
  // first
  for (int point_idx = 0; point_idx < num_local_points; ++point_idx) {
    for (int const &corner_idx : p_to_c_map[point_idx]) {
      mp_to_c_map.push_back(corner_idx);
      corner_count++;
    }
    mp_to_c_count[point_idx + 1] = corner_count;
  }

  const UmeVector<size_t> mc_to_z_map(begin(c_to_z_map), end(c_to_z_map));
#ifdef USE_CALI
  CALI_MARK_END("Gradzatp_Invert_Scoria_Map");
#endif

#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatp_Invert_Point_Volume_Scoria_Pack");
#endif
  DBLV_T packed_cv(corner_count, 0.0);
  DBLV_T packed_zf(corner_count, 0.0);
  DBLV_T packed_csurf_x(corner_count, 0.0);
  DBLV_T packed_csurf_y(corner_count, 0.0);
  DBLV_T packed_csurf_z(corner_count, 0.0);

  DBLV_T csurf_x(csurf.size(), 0.0);
  DBLV_T csurf_y(csurf.size(), 0.0);
  DBLV_T csurf_z(csurf.size(), 0.0);

for (size_t i = 0; i < csurf.size(); i += TILE_SIZE) {
    for (size_t j = i; j < std::min(TILE_SIZE+i,csurf.size()); j++) {
    csurf_x[j] = csurf[j][0];
    csurf_y[j] = csurf[j][1];
    csurf_z[j] = csurf[j][2];
    }
  }

  /*for (size_t i = 0; i < csurf.size(); ++i) {
    csurf_x[i] = csurf[i][0];
    csurf_y[i] = csurf[i][1];
    csurf_z[i] = csurf[i][2];
  }*/
#ifdef USE_CALI
  CALI_MARK_END("Gradzatp_Invert_Point_Volume_Scoria_Pack");
#endif

#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatp_Invert_Point_Volume_Scoria_Read");
#endif
#ifdef USE_CLIENT
  struct request req1;
  scoria_read(client, corner_volume.data(), corner_count, packed_cv.data(),
      mp_to_c_map.data(), NULL, num_threads, NONE, &req1);
  scoria_wait_request(client, &req1);

  struct request req2;
  scoria_read(client, zone_field.data(), corner_count, packed_zf.data(),
      mp_to_c_map.data(), mc_to_z_map.data(), num_threads, NONE, &req2);
  scoria_wait_request(client, &req2);

  struct request req3;
  scoria_read(client, csurf_x.data(), corner_count, packed_csurf_x.data(),
      mp_to_c_map.data(), NULL, num_threads, NONE, &req3);
  scoria_wait_request(client, &req3);

  struct request req4;
  scoria_read(client, csurf_y.data(), corner_count, packed_csurf_y.data(),
      mp_to_c_map.data(), NULL, num_threads, NONE, &req4);
  scoria_wait_request(client, &req4);

  struct request req5;
  scoria_read(client, csurf_z.data(), corner_count, packed_csurf_z.data(),
      mp_to_c_map.data(), NULL, num_threads, NONE, &req5);
  scoria_wait_request(client, &req5);
#else
  read_multi_thread_1(packed_cv.data(), corner_volume.data(), corner_count,
      mp_to_c_map.data(), num_threads, NONE);
  read_multi_thread_2(packed_zf.data(), zone_field.data(), corner_count,
      mp_to_c_map.data(), mc_to_z_map.data(), num_threads, NONE);
  read_multi_thread_1(packed_csurf_x.data(), csurf_x.data(), corner_count,
      mp_to_c_map.data(), num_threads, NONE);
  read_multi_thread_1(packed_csurf_y.data(), csurf_y.data(), corner_count,
      mp_to_c_map.data(), num_threads, NONE);
  read_multi_thread_1(packed_csurf_z.data(), csurf_z.data(), corner_count,
      mp_to_c_map.data(), num_threads, NONE);
#endif
#ifdef USE_CALI
  CALI_MARK_END("Gradzatp_Invert_Point_Volume_Scoria_Read");
#endif
#endif

  DBLV_T point_volume(num_points, 0.0);
  point_gradient.assign(num_points, VEC3_T(0.0));

#ifdef USE_SCORIA
#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatp_Invert_Point_Volume_Scoria_Pack");
#endif
  DBLV_T point_gradient_x(point_gradient.size(), 0.0);
  DBLV_T point_gradient_y(point_gradient.size(), 0.0);
  DBLV_T point_gradient_z(point_gradient.size(), 0.0);
#ifdef USE_CALI
  CALI_MARK_END("Gradzatp_Invert_Point_Volume_Scoria_Pack");
#endif

#ifdef USE_CALI
  CALI_CXX_MARK_LOOP_BEGIN(gradzatp_invert_point_volume_scoria_loop_id,
      "Gradzatp_Invert_Point_Volume_Loop");
#endif
  // std::cout<<"local_points is "<<num_local_points<<std::endl;
  // Same as the previous comment
  for (int point_idx = 0; point_idx < num_local_points; ++point_idx) {
    for (size_t c = mp_to_c_count[point_idx]; c < mp_to_c_count[point_idx + 1];
         ++c) {
      point_volume[point_idx] += packed_cv[c];

      point_gradient_x[point_idx] += packed_csurf_x[c] * packed_zf[c];
      point_gradient_y[point_idx] += packed_csurf_y[c] * packed_zf[c];
      point_gradient_z[point_idx] += packed_csurf_z[c] * packed_zf[c];
    }
  }
#ifdef USE_CALI
  CALI_CXX_MARK_LOOP_END(gradzatp_invert_point_volume_scoria_loop_id);
#endif

#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatp_Invert_Point_Volume_Scoria_Unpack");
#endif

  for (size_t i = 0; i < point_gradient.size(); i += TILE_SIZE) {
    for (size_t j = i; j < std::min(TILE_SIZE+i,point_gradient.size()); j++) {
    point_gradient[j][0] = point_gradient_x[j];
    point_gradient[j][1] = point_gradient_y[j];
    point_gradient[j][2] = point_gradient_z[j];
    }
  }

  /*for (size_t i = 0; i < point_gradient.size(); ++i) {
    point_gradient[i][0] = point_gradient_x[i];
    point_gradient[i][1] = point_gradient_y[i];
    point_gradient[i][2] = point_gradient_z[i];
  }*/
#ifdef USE_CALI
  CALI_MARK_END("Gradzatp_Invert_Point_Volume_Scoria_Unpack");
#endif
#else
  point_gradient.assign(num_points, VEC3_T(0.0));
#ifdef USE_CALI
  CALI_CXX_MARK_LOOP_BEGIN(gradzatp_invert_point_volume_base_loop_id,
      "Gradzatp_Invert_Point_Volume_Loop");
#endif
  for (int point_idx = 0; point_idx < num_local_points; ++point_idx) {
    for (int const &corner_idx : p_to_c_map[point_idx]) {
      int const zone_idx = c_to_z_map[corner_idx];
      point_volume[point_idx] += corner_volume[corner_idx];
      point_gradient[point_idx] += csurf[corner_idx] * zone_field[zone_idx];
    }
  }
#ifdef USE_CALI
  CALI_CXX_MARK_LOOP_END(gradzatp_invert_point_volume_base_loop_id);
#endif
#endif

#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatp_Invert_GathScat");
#endif
  mesh.points.gathscat(Ume::Comm::Op::SUM, point_volume);
  mesh.points.gathscat(Ume::Comm::Op::SUM, point_gradient);
#ifdef USE_CALI
  CALI_MARK_END("Gradzatp_Invert_GathScat");
#endif

  /*
    Divide by point control volume to get gradient.  If a point is on the outer
    perimeter of the mesh (POINT_TYPE=-1), subtract the outward normal component
    of the gradient using the point normals.
   */
#ifdef USE_CALI
  CALI_CXX_MARK_LOOP_BEGIN(gradzatp_invert_point_gradient_base_loop_id,
      "Gradzatp_Invert_Point_Gradient_Loop");
#endif

  for (int point_idx = 0; point_idx < num_local_points;
       point_idx += TILE_SIZE) {
    for (int j = point_idx;
         j < std::min(point_idx + TILE_SIZE, num_local_points); j++) {
      if (point_type[j] > 0) {
        // Internal point
        point_gradient[j] /= point_volume[j];
      } else if (point_type[j] == -1) {
        // Mesh boundary point
        double const ppdot = dotprod(point_gradient[j], point_normal[j]);
        point_gradient[j] =
            (point_gradient[j] - point_normal[j] * ppdot) / point_volume[j];
      }
    }
  }

  /*for (int point_idx = 0; point_idx < num_local_points; ++point_idx) {
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
  }*/

#ifdef USE_CALI
  CALI_CXX_MARK_LOOP_END(gradzatp_invert_point_gradient_base_loop_id);
#endif

#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatp_Invert_Scatter");
#endif
  mesh.points.scatter(point_gradient);
#ifdef USE_CALI
  CALI_MARK_END("Gradzatp_Invert_Scatter");
#endif
}

#if defined(USE_SCORIA) && defined(USE_CLIENT)
void gradzatz_invert(struct client *client, Ume::SOA_Idx::Mesh &mesh,
    DBLV_T const &zone_field, VEC3V_T &zone_gradient, VEC3V_T &point_gradient) {
#else
void gradzatz_invert(Ume::SOA_Idx::Mesh &mesh, DBLV_T const &zone_field,
    VEC3V_T &zone_gradient, VEC3V_T &point_gradient) {
#endif

#ifdef USE_CALI
  CALI_CXX_MARK_FUNCTION;
#endif

  auto const &z_to_c_map = mesh.ds->caccess_intrr("m:z>c");
  auto const &c_to_p_map = mesh.ds->caccess_intv("m:c>p");
  int const num_local_zones = mesh.zones.local_size();
  auto const &zone_type = mesh.zones.mask;
  auto const &corner_volume = mesh.ds->caccess_dblv("corner_vol");

#ifdef USE_SCORIA
#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatz_Invert_Scoria_Map");
#endif
  const int num_threads = NTHREADS;

  UmeVector<size_t> mz_to_c_map;
  UmeVector<size_t> mz_to_c_count(num_local_zones + 1);

  int corner_count = 0;
  mz_to_c_count[0] = 0;
  for (int zone_idx = 0; zone_idx < num_local_zones; zone_idx += TILE_SIZE) {
    for (int j = zone_idx; j < std::min(zone_idx + TILE_SIZE, num_local_zones);
         j++) {
      for (int corner_idx : z_to_c_map[j]) {
        mz_to_c_map.push_back(corner_idx);
        corner_count++;
      }

      mz_to_c_count[j + 1] = corner_count;
    }
  }

  /*for (int zone_idx = 0; zone_idx < num_local_zones; ++zone_idx) {
    for (int corner_idx : z_to_c_map[zone_idx]) {
      mz_to_c_map.push_back(corner_idx);
      corner_count++;
    }
    mz_to_c_count[zone_idx + 1] = corner_count;
  }*/
  // std::cout<<"mztocmap size: "<<mz_to_c_map.size()<<std::endl;

  const UmeVector<size_t> mc_to_p_map(begin(c_to_p_map), end(c_to_p_map));
#ifdef USE_CALI
  CALI_MARK_END("Gradzatz_Invert_Scoria_Map");
#endif
#endif

  // Get the field gradient at each mesh point.
#if defined(USE_SCORIA) && defined(USE_CLIENT)
  gradzatp_invert(client, mesh, zone_field, point_gradient);
#else
  gradzatp_invert(mesh, zone_field, point_gradient);
#endif

  zone_gradient.assign(mesh.zones.size(), VEC3_T(0.0));

#ifdef USE_SCORIA
#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatz_Invert_Zone_Gradient_Scoria_Pack");
#endif
  DBLV_T packed_cv(corner_count, 0.0);
  DBLV_T packed_pg_x(corner_count, 0.0);
  DBLV_T packed_pg_y(corner_count, 0.0);
  DBLV_T packed_pg_z(corner_count, 0.0);

  DBLV_T point_gradient_x(point_gradient.size(), 0.0);
  DBLV_T point_gradient_y(point_gradient.size(), 0.0);
  DBLV_T point_gradient_z(point_gradient.size(), 0.0);
  
  
  for (size_t i = 0; i < point_gradient.size(); i += TILE_SIZE) {
    for (size_t j = i; j < std::min(TILE_SIZE+i,point_gradient.size()); j++) {
    point_gradient_x[j] = point_gradient[j][0];
    point_gradient_y[j] = point_gradient[j][1];
    point_gradient_z[j] = point_gradient[j][2];
    }
  }

  /*for (size_t i = 0; i < point_gradient.size(); ++i) {
    point_gradient_x[i] = point_gradient[i][0];
    point_gradient_y[i] = point_gradient[i][1];
    point_gradient_z[i] = point_gradient[i][2];
  }*/

#ifdef USE_CALI
  CALI_MARK_END("Gradzatz_Invert_Zone_Gradient_Scoria_Pack");
#endif

#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatz_Invert_Zone_Gradient_Scoria_Read");
#endif
#ifdef USE_CLIENT
  struct request req1;
  scoria_read(client, corner_volume.data(), corner_count, packed_cv.data(),
      mz_to_c_map.data(), NULL, num_threads, NONE, &req1);
  scoria_wait_request(client, &req1);

  struct request req2;
  scoria_read(client, point_gradient_x.data(), corner_count, packed_pg_x.data(),
      mz_to_c_map.data(), mc_to_p_map.data(), num_threads, NONE, &req2);
  scoria_wait_request(client, &req2);

  struct request req3;
  scoria_read(client, point_gradient_y.data(), corner_count, packed_pg_y.data(),
      mz_to_c_map.data(), mc_to_p_map.data(), num_threads, NONE, &req2);
  scoria_wait_request(client, &req3);

  struct request req4;
  scoria_read(client, point_gradient_z.data(), corner_count, packed_pg_z.data(),
      mz_to_c_map.data(), mc_to_p_map.data(), num_threads, NONE, &req2);
  scoria_wait_request(client, &req4);
#else
  read_multi_thread_1(packed_cv.data(), corner_volume.data(), corner_count,
      mz_to_c_map.data(), num_threads, NONE);
  read_multi_thread_2(packed_pg_x.data(), point_gradient_x.data(), corner_count,
      mz_to_c_map.data(), mc_to_p_map.data(), num_threads, NONE);
  read_multi_thread_2(packed_pg_y.data(), point_gradient_y.data(), corner_count,
      mz_to_c_map.data(), mc_to_p_map.data(), num_threads, NONE);
  read_multi_thread_2(packed_pg_z.data(), point_gradient_z.data(), corner_count,
      mz_to_c_map.data(), mc_to_p_map.data(), num_threads, NONE);
#endif
#ifdef USE_CALI
  CALI_MARK_END("Gradzatz_Invert_Zone_Gradient_Scoria_Read");
#endif

#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatz_Invert_Zone_Gradient_Scoria_Pack");
#endif
  DBLV_T zone_gradient_x(zone_gradient.size(), 0.0);
  DBLV_T zone_gradient_y(zone_gradient.size(), 0.0);
  DBLV_T zone_gradient_z(zone_gradient.size(), 0.0);
#ifdef USE_CALI
  CALI_MARK_END("Gradzatz_Invert_Zone_Gradient_Scoria_Pack");
#endif

#ifdef USE_CALI
  CALI_CXX_MARK_LOOP_BEGIN(gradzatz_invert_zone_gradient_scoria_loop_id,
      "Gradzatz_Invert_Zone_Gradient_Loop");
#endif
  // std::cout<<"Local zones is: "<<num_local_zones <<std::endl;

  // Need to split the loop if tiling is needed then.

  std::vector<double> zone_volume(num_local_zones, 0);

  for (int zone_idx = 0; zone_idx < num_local_zones; ++zone_idx) {
    if (zone_type[zone_idx] < 1)
      continue;

    // double zone_volume{0.0};
    for (size_t c = mz_to_c_count[zone_idx]; c < mz_to_c_count[zone_idx + 1];
         ++c) {
      zone_volume[zone_idx] += packed_cv[c];
    }
  }

  for (int zone_idx = 0; zone_idx < num_local_zones; zone_idx += TILE_SIZE) {
    for (int j = zone_idx; j < std::min(TILE_SIZE + zone_idx, num_local_zones);
         j++) {
      if (zone_type[j] < 1)
        continue;

      for (size_t c = mz_to_c_count[j]; c < mz_to_c_count[j + 1]; ++c) {
        double const c_z_vol_ratio = packed_cv[c] / zone_volume[j];
        zone_gradient_x[j] += packed_pg_x[c] * c_z_vol_ratio;
        zone_gradient_y[j] += packed_pg_y[c] * c_z_vol_ratio;
        zone_gradient_z[j] += packed_pg_z[c] * c_z_vol_ratio;
      }
    }
  }

#ifdef USE_CALI
  CALI_CXX_MARK_LOOP_END(gradzatz_invert_zone_gradient_scoria_loop_id);
#endif

#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatz_Invert_Zone_Gradient_Scoria_Unpack");
#endif

  for (size_t i = 0; i < zone_gradient.size(); i += TILE_SIZE) {
    for (size_t j = i; j < std::min(TILE_SIZE+i,zone_gradient.size()); j++) {
    zone_gradient[j][0] = zone_gradient_x[j];
    zone_gradient[j][1] = zone_gradient_y[j];
    zone_gradient[j][2] = zone_gradient_z[j];
    }
  }

  /*
  for (size_t i = 0; i < zone_gradient.size(); ++i) {
    zone_gradient[i][0] = zone_gradient_x[i];
    zone_gradient[i][1] = zone_gradient_y[i];
    zone_gradient[i][2] = zone_gradient_z[i];
  }
  */
#ifdef USE_CALI
  CALI_MARK_END("Gradzatz_Invert_Zone_Gradient_Scoria_Unpack");
#endif
#else
#ifdef USE_CALI
  CALI_CXX_MARK_LOOP_BEGIN(gradzatz_invert_zone_gradient_base_loop_id,
      "Gradzatz_Invert_Zone_Gradient_Loop");
#endif

  std::vector<double> zone_volume(num_local_zones, 0);
  for (int zone_idx = 0; zone_idx < num_local_zones; ++zone_idx) {
    if (zone_type[zone_idx] < 1)
      continue; // Only operate on local interior zones

    // Accumulate the (local) zone volume
    // double zone_volume{0.0}; // Only need a local volume
    for (int const &corner_idx : z_to_c_map[zone_idx]) {
      zone_volume[zone_idx] += corner_volume[corner_idx];
    }
  }

  for (int zone_idx = 0; zone_idx < num_local_zones; zone_idx += TILE_SIZE) {
    for (int j = zone_idx; j < std::min(TILE_SIZE + zone_idx, num_local_zones);
         j++) {
      if (zone_type[j] < 1)
        continue;
      for (int const &corner_idx : z_to_c_map[j]) {
        int const point_idx = c_to_p_map[corner_idx];
        double const c_z_vol_ratio = corner_volume[corner_idx] / zone_volume[j];
        zone_gradient[j] += point_gradient[point_idx] * c_z_vol_ratio;
      }
    }
  }
#ifdef USE_CALI
  CALI_CXX_MARK_LOOP_END(gradzatz_invert_zone_gradient_base_loop_id);
#endif
#endif

#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatz_Invert_Scatter");
#endif
  mesh.zones.scatter(zone_gradient);
#ifdef USE_CALI
  CALI_MARK_END("Gradzatz_Invert_Scatter");
#endif
}

} // namespace Ume
