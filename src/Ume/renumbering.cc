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
  \file Ume/renumbering.cc
*/

#include "Ume/renumbering.hh"

namespace Ume {

using Mesh = SOA_Idx::Mesh;
using INTV_T = DS_Types::INTV_T;

enum OPS {
  MIN = 1,
  MAX,
  NONE
};
typedef int Op_t;

void renumber_mesh(Mesh &mesh) {
  renumber_s_maps(mesh);
}

void renumber_s_maps(Mesh &mesh) {
  /* Get sizes for general use. */
  int const sl = mesh.sides.local_size();
  int const sll = mesh.sides.size();
  int const sgl = mesh.sides.ghost_local_size();
  int const sgll = mesh.sides.ghost_size();
  int const pl = mesh.points.local_size();

  /* Initialize new indices to current indices. */
  INTV_T s_to_snew_map(sll, 0);
  for (int s : mesh.sides.all_indices())
    s_to_snew_map[s] = s;

  /* Initialize new ghost indices to current ghost indices. */
  INTV_T sg_to_sgnew_map(sgll, 0);
  for (int sg : mesh.sides.ghost_indices_offset()) {
    sg_to_sgnew_map[sg] = sg;
  }

  /* Create new s->p mappings. */
  INTV_T s_to_p_map_new(sll, 0);
  auto const &side_type = mesh.sides.mask;
  auto const &ghost_side_type = mesh.sides.ghost_mask;
  auto const &s_to_p1_map = mesh.ds->caccess_intv("m:s>p1");
  auto const &s_to_p2_map = mesh.ds->caccess_intv("m:s>p2");
  auto const &sg_to_s_map = mesh.sides.cpy_idx;

  Op_t op = MIN;
  do { // First sort by min then by max
    for (int s : mesh.sides.all_indices()) {
      if (side_type[s] == 0)
        continue; // Ignore null sides

      int const snew = s_to_snew_map[s];
      int const p1 = s_to_p1_map[s];
      int const p2 = s_to_p2_map[s];

      if (MIN == op) {
        s_to_p_map_new[snew] = std::min(p1, p2);
      } else if (MAX == op) {
        s_to_p_map_new[snew] = std::max(p1, p2);
      }
    }

    int s_max = 0;
    INTV_T snew_to_snew2_map(sll, 0);
    new_numbering(sl, side_type,
                  pl, s_to_p_map_new,
                  s_max, snew_to_snew2_map);

    for (int s : mesh.sides.all_indices()) { // Set new indices
      int const snew = s_to_snew_map[s];
      if (snew == 0)
        continue;

      int const snew2 = snew_to_snew2_map[snew];
      s_to_snew_map[s] = snew2;
    }

    if (sgl > 0) { // Set new ghost indices, if there are ghosts
      for (int sg : mesh.sides.ghost_indices_offset()) {
        if (ghost_side_type[sg] == 0)
          continue;

        int const s = sg_to_s_map[sg];
        s_max += 1;
        s_to_snew_map[s] = s_max;
      }
    }

    op += 1;
  } while(op <= MAX);
}

void new_numbering(int const xl, auto const &x_type,
                   int const yl, INTV_T const &x_to_y_map,
                   int &x_max, INTV_T &x_to_xnew_map) {
  int const yl1 = yl + 1;
  INTV_T storage_locations(yl1, 0);

  for (int x = 0; x < xl; ++x) { // Count x attached to each y
    if (x_type[x] == 0)
      continue;

    int const y = x_to_y_map[x];
    storage_locations[y + 1] += 1;
  }

  for (int y = 1; y < yl1; ++y) { // Sum storage locations
    storage_locations[y] += storage_locations[y - 1];
  }

  for (int x = 0; x < xl; ++x) { // Set new numbers
    if (x_type[x] == 0)
      continue;

    int const y = x_to_y_map[x];
    x_to_xnew_map[x] = storage_locations[y];
    x_max = std::max(x_max, storage_locations[y]);
  }
}

} // namespace Ume
