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

/* Renumber_MeshMaps[RenumWaveMinMax]-->RenumWaveMinMax */
void renumber_mesh(Mesh &mesh) {
  /* Broadcast each to the mesh handle. */
  renumber_s_maps(mesh);
}

/* Renumber_S[kksll]-->Renumb_S */
void renumber_s_maps(Mesh &mesh) {
  /* Get sizes for general use. */
  int const sll = mesh.sides.size();
  int const sgl = mesh.sides.ghost_local_size();
  int const sgll = mesh.sides.ghost_size();

  /* Store new-to-old mappings for debugging/testing. After the reshape,
   * these will represent current-to-old mappings. */
  INTV_T s_to_sold_map(sll, 0);
  for (int s : mesh.sides.all_indices())
    s_to_sold_map[s] = s;

  /* Initialize local storage for new mappings. */
  INTV_T s_to_snew_map(sll, 0);
  INTV_T sg_to_sgnew_map(sgll, 0);

  { /* Renumber_SMaps[RenumWaveMinMax]-->RenumWaveMinMaxS */
    /* Initialize new indices to current indices. */
    for (int s : mesh.sides.all_indices())
      s_to_snew_map[s] = s;

    Op_t op = MIN;
    do { /* Sides_minmax */
      /* Access database. */
      auto const &side_type = mesh.sides.mask;
      auto const &ghost_side_type = mesh.sides.ghost_mask;
      auto const &s_to_p1_map = mesh.ds->caccess_intv("m:s>p1");
      auto const &s_to_p2_map = mesh.ds->caccess_intv("m:s>p2");
      auto const &sg_to_s_map = mesh.sides.cpy_idx;

      /* Use simple map for ghosts. */
      for (int sg : mesh.sides.ghost_indices_offset())
        sg_to_sgnew_map[sg] = sg;

      /* Create new s->p mappings. */
      INTV_T s_to_p_map_new(sll, 0);

      /* Fill help array based on min/max point number and flag. */
      for (int s : mesh.sides.local_indices()) {
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

      /* Generate the new numbers. */
      int s_max = 0;
      INTV_T snew_to_snew2_map(sll, 0);
      new_numbering(mesh.sides, mesh.points,
                    s_to_p_map_new,
                    s_max, snew_to_snew2_map);

      /* The first iteration translates X->XNEW and the second iteration
       * translates XNEW->XNEW2. The X->XNEW map contains both
       * translations X->XNEW->XNEW2. */
      for (int s : mesh.sides.all_indices()) {
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

  /* Now that we have new local maps, we need to reshape db arrays for
   * this entity and index lists pointing to this entity data. This is
   * a special case of a general ReshapeX operation since the sizes of
   * the entity data have not changed, only the ordering. */

  { /* ReshapeS() */
    /* Set masks to 1 or 0 depending on map value */
    INTV_T side_type_new(sll, 0);
    for (int s: mesh.sides.all_indices()) {
      side_type_new[s] = std::max(0, std::min(s_to_snew_map[s], 1));
      s_to_snew_map[s] = std::abs(s_to_snew_map[s]);
    }

    INTV_T ghost_side_type_new(sgll, 0);
    auto const &ghost_side_type = mesh.sides.ghost_mask;
    if (sgl > 0) {
      for (int sg : mesh.sides.ghost_indices_offset()) {
        if (ghost_side_type[sg] == 0)
          continue;
        ghost_side_type_new[sg] = std::max(0, std::min(sg_to_sgnew_map[sg], 1));
        sg_to_sgnew_map[sg] = std::abs(sg_to_sgnew_map[sg]);
      }
    }

    /* Update MPI stuff: upKKSSLVPELL() */

    /* We must now renumber all the database maps and variables for
     * this entity. To do this, we emulate the broadcasts to mesh
     * entities of the form m:x>y and m:y>x along with all variables
     * beginning with the letter x. */

    /* ReshapeS[Mesh]-->(potentially lots of things) */

    { /* Reshape m:y>x maps. */
      std::vector<std::string> entity_names;
      mesh.ds->to_entity_map_names("s", entity_names);

      for (size_t x = 0; x < entity_names.size(); ++x) {
      }
    }

    { /* Reshape m:x>y maps. */
      std::vector<std::string> entity_names;
      mesh.ds->from_entity_map_names("s", entity_names);

      for (size_t x = 0; x < entity_names.size(); ++x) {
      }
    }

    { /* Reshape x vars. */
      std::vector<std::string> entity_names;
      mesh.ds->entity_var_names("s", entity_names);

      for (size_t x = 0; x < entity_names.size(); ++x) {
      }
    }
  }
}

} // namespace Ume
