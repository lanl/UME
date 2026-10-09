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
  \file Ume/face_area.cc
*/

#include "Ume/face_area.hh"
#include "Ume/atomic_add.hh"
#include "Ume/mem_exec_spaces.hh"

namespace Ume {

using Mesh = SOA_Idx::Mesh;
using DBLV_T = DS_Types::DBLV_T;

void calc_face_area(Mesh &mesh, DBLV_T &face_area) {
  auto const &side_type = mesh.sides.mask;
  auto const &face_comm_type = mesh.faces.comm_type;
  auto const &s_to_f_map = mesh.ds->caccess_intv("m:s>f");
  auto const &s_to_s2_map = mesh.ds->caccess_intv("m:s>s2");
  auto const &surz = mesh.ds->caccess_vec3v("side_surz");

  int const sl = mesh.sides.local_size();

  std::fill(face_area.begin(), face_area.end(), 0.0);

  Kokkos::View<double *, HostSpace> h_face_area(
      face_area.data(), face_area.size());
  Kokkos::View<const int *, HostSpace> h_s_to_f_map(
      s_to_f_map.data(), s_to_f_map.size());
  Kokkos::View<const int *, HostSpace> h_s_to_s2_map(
      s_to_s2_map.data(), s_to_s2_map.size());
  Kokkos::View<const Vec3 *, HostSpace> h_surz(surz.data(), surz.size());
  Kokkos::View<const short *, HostSpace> h_side_type(
      side_type.data(), side_type.size());
  Kokkos::View<const int *, HostSpace> h_face_comm_type(
      face_comm_type.data(), face_comm_type.size());

  auto d_face_area =
      Kokkos::create_mirror_view_and_copy(DevExecMemSpace(), h_face_area);
  auto d_s_to_f_map =
      Kokkos::create_mirror_view_and_copy(DevExecMemSpace(), h_s_to_f_map);
  auto d_s_to_s2_map =
      Kokkos::create_mirror_view_and_copy(DevExecMemSpace(), h_s_to_s2_map);
  auto d_surz = Kokkos::create_mirror_view_and_copy(DevExecMemSpace(), h_surz);
  auto d_side_type =
      Kokkos::create_mirror_view_and_copy(DevExecMemSpace(), h_side_type);
  auto d_face_comm_type =
      Kokkos::create_mirror_view_and_copy(DevExecMemSpace(), h_face_comm_type);

  Kokkos::parallel_for(
      "face_area", Kokkos::RangePolicy<DevExecSpace>(0, sl),
      KOKKOS_LAMBDA(const int s) {
        /* A side and its partner s2 cover the same part of the face, so take
           the lower-numbered of a real pair, or the real one if s2 is not; a
           side that is its own partner is the only one.  The pair relation is
           static and symmetric, so the choice does not depend on which of the
           two another thread has reached. */
        int const s2 = d_s_to_s2_map(s);
        if (d_side_type(s) >= 1 && (s <= s2 || d_side_type(s2) < 1)) {
          int const f = d_s_to_f_map(s);
          if (d_face_comm_type(f) < 3) { // Internal or master face
            double const side_area = vectormag(d_surz(s)); // Flat area
            ume_atomic_add(&d_face_area(f), side_area);
          }
        }
      });

  Kokkos::deep_copy(h_face_area, d_face_area);

  mesh.faces.scatter(face_area);
}

} // namespace Ume
