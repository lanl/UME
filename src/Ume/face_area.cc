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

#ifdef USE_CALI
#include <caliper/cali-manager.h>
#include <caliper/cali.h>
#endif

#include "Ume/face_area.hh"

namespace Ume {

using Mesh = SOA_Idx::Mesh;
using DBLV_T = DS_Types::DBLV_T;
using INTV_T = DS_Types::INTV_T;
using VEC3V_T = DS_Types::VEC3V_T;

void calc_face_area(Mesh &mesh, DBLV_T &face_area, int cali_record) {
  auto const &side_type = mesh.sides.mask;
  auto const &face_comm_type = mesh.faces.comm_type;
  auto const &s_to_f_map = mesh.ds->caccess_intv("m:s>f");
  auto const &s_to_s2_map = mesh.ds->caccess_intv("m:s>s2");
  auto const &surz = mesh.ds->caccess_vec3v("side_surz");

  int const sll = mesh.sides.size();
  int const sl = mesh.sides.local_size();

  std::fill(face_area.begin(), face_area.end(), 0.0);
  INTV_T side_tag(sll, 0);

#ifdef USE_CALI
  if(cali_record)
    CALI_MARK_BEGIN("Calc_Face_Area_Loop");
#endif
  for (int s = 0; s < sl; ++s) {
    if (side_type[s] < 1)
      continue; // We want internal sides only
    if (side_tag[s] == 1)
      continue; // Already added this side via s2

    int const f = s_to_f_map[s];
    if (face_comm_type[f] < 3) { // Internal or master face
      double const side_area = vectormag(surz[s]); // Flat area
      face_area[f] += side_area;

      int const s2 = s_to_s2_map[s];
      side_tag[s2] = 1;
    }
  }
#ifdef USE_CALI
  if(cali_record)
    CALI_MARK_END("Calc_Face_Area_Loop");
#endif
  mesh.faces.scatter(face_area);
}

} // namespace Ume
