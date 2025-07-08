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
  renumber_p(mesh);
  renumber_s(mesh);
  renumber_z(mesh);
  renumber_f(mesh);
  renumber_e(mesh);
  renumber_c(mesh);
}

/* Renumber_P[kkpll]-->Renumb_P */
void renumber_p(Mesh &mesh) {
  /* Get sizes for general use. */
  int const pll = mesh.points.size();
  int const pgl = mesh.points.ghost_local_size();
  int const pgll = mesh.points.ghost_size();

  /* Store new-to-old mappings for debugging/testing. After the reshape,
   * these will represent current-to-old mappings. */
  INTV_T p_to_pold_map(pll, 0);
  for (int p : mesh.points.all_indices())
    p_to_pold_map[p] = p;

  /* Initialize local storage for new mappings. */
  INTV_T p_to_pnew_map(pll, 0);
  INTV_T pg_to_pgnew_map(pgll, 0);
}

/* Renumber_S[kksll]-->Renumb_S */
void renumber_s(Mesh &mesh) {
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

/* Renumber_Z[kkzll]-->Renumb_Z */
void renumber_z(Mesh &mesh) {
  /* Get sizes for general use. */
  int const zll = mesh.zones.size();
  int const zgl = mesh.zones.ghost_local_size();
  int const zgll = mesh.zones.ghost_size();

  /* Store new-to-old mappings for debugging/testing. After the reshape,
   * these will represent current-to-old mappings. */
  INTV_T z_to_zold_map(zll, 0);
  for (int z : mesh.zones.all_indices())
    z_to_zold_map[z] = z;

  /* Initialize local storage for new mappings. */
  INTV_T z_to_znew_map(zll, 0);
  INTV_T zg_to_zgnew_map(zgll, 0);

  { /* Renumber_ZMaps[RenumWaveMinMax]-->RenumWaveMinMaxZ */
    /* Initialize new indices to current indices. */
    for (int z : mesh.zones.all_indices())
      z_to_znew_map[z] = z;

    Op_t op = MIN;
    do { /* Zones_minmax */
      /* Access database. */
      auto const &side_type = mesh.sides.mask;
      auto const &ghost_zone_type = mesh.zones.ghost_mask;
      auto const &s_to_z_map = mesh.ds->caccess_intv("m:s>z");
      auto const &s_to_p1_map = mesh.ds->caccess_intv("m:s>p1");
      auto const &s_to_p2_map = mesh.ds->caccess_intv("m:s>p2");
      auto const &zg_to_z_map = mesh.zones.cpy_idx;

      /* Use simple map for ghosts. */
      for (int zg : mesh.zones.ghost_indices_offset())
        zg_to_zgnew_map[zg] = zg;

      /* Create new z->p mappings. */
      INTV_T z_to_p_map_new(zll, 0);

      /* Set the initial point number for each zone to be  either zero
       * (for max sort) or 2*kkpll (for min sort). */
      int p = 0;

      if (MIN == op)
        p = 2*mesh.points.size();

      for (int z : mesh.zones.local_indices()) {
        z_to_p_map_new[z] = p;
      }

      /* Identify the min/max point number on each zone. */
      for (int s : mesh.sides.local_indices()) {
        if (side_type[s] == 0)
          continue;

        int const z = s_to_z_map[s];
        int const p1 = s_to_p1_map[s];
        int const p2 = s_to_p2_map[s];
        int const znew = z_to_znew_map[z];
        int const p = z_to_p_map_new[znew];

        if (MIN == op) {
          int const temp = std::min(p1, p2);
          z_to_p_map_new[znew] = std::min(temp, p);
        } else if (MAX == op) {
          int const temp = std::max(p1, p2);
          z_to_p_map_new[znew] = std::max(temp, p);
        }
      }

      /* Generate the new numbers. */
      int z_max = 0;
      INTV_T znew_to_znew2_map(zll, 0);
      new_numbering(mesh.zones, mesh.points,
                    z_to_p_map_new,
                    z_max, znew_to_znew2_map);

      /* The first iteration translates X->XNEW and the second iteration
       * translates XNEW->XNEW2. The X->XNEW map contains both
       * translations X->XNEW->XNEW2. */
      for (int z : mesh.zones.all_indices()) {
        int const znew = z_to_znew_map[z];
        if (znew == 0)
          continue;

        int const znew2 = znew_to_znew2_map[znew];
        z_to_znew_map[z] = znew2;
      }

      if (zgl > 0) { // Set new ghost indices, if there are ghosts
        for (int zg : mesh.zones.ghost_indices_offset()) {
          if (ghost_zone_type[zg] == 0)
            continue;

          int const z = zg_to_z_map[zg];
          z_max += 1;
          z_to_znew_map[z] = z_max;
        }
      }

      op += 1;
    } while(op <= MAX);
  }

  { /* ReshapeZ() */
  }
}

/* Renumber_F[kkfll]-->Renumb_F */
void renumber_f(Mesh &mesh) {
  /* Get sizes for general use. */
  int const fll = mesh.faces.size();
  int const fgl = mesh.faces.ghost_local_size();
  int const fgll = mesh.faces.ghost_size();

  /* Store new-to-old mappings for debugging/testing. After the reshape,
   * these will represent current-to-old mappings. */
  INTV_T f_to_fold_map(fll, 0);
  for (int f : mesh.faces.all_indices())
    f_to_fold_map[f] = f;

  /* Initialize local storage for new mappings. */
  INTV_T f_to_fnew_map(fll, 0);
  INTV_T fg_to_fgnew_map(fgll, 0);

  { /* Renumber_FMaps[RenumWaveMinMax]-->RenumWaveMinMaxF */
    /* Faces get renumbered by minimum side number (only have to do it
     * once because there is never more than one face attached to a
     * side). */
    { /* Faces_minmax */
      /* Access database. */
      auto const &side_type = mesh.sides.mask;
      auto const &s_to_f_map = mesh.ds->caccess_intv("m:s>f");
      auto const &s_to_s2_map = mesh.ds->caccess_intv("m:s>s2");

      /* Use simple map for ghosts. */
      for (int fg : mesh.faces.ghost_indices_offset())
        fg_to_fgnew_map[fg] = fg;

      /* Create new f->s mappings. */
      INTV_T f_to_s_map_new(fll, mesh.sides.size() + 1);
      for (int s : mesh.sides.local_indices()) {
        if (side_type[s] == 0)
          continue;

        int const f = s_to_f_map[s];
        int const s2 = s_to_s2_map[s];
        int const temp = std::min(s, s2);
        f_to_s_map_new[f] = std::min(temp, f_to_s_map_new[f]);
      }

      /* Generate the new numbers. */
      int f_max = 0;
      new_numbering(mesh.faces, mesh.sides,
                    f_to_s_map_new,
                    f_max, f_to_fnew_map);
    }
  }

  { /* ReshapeF() */
  }
}

/* Renumber_E[kkell]-->Renumb_E */
void renumber_e(Mesh &mesh) {
  /* Get sizes for general use. */
  int const ell = mesh.edges.size();
  int const egl = mesh.edges.ghost_local_size();
  int const egll = mesh.edges.ghost_size();

  /* Store new-to-old mappings for debugging/testing. After the reshape,
   * these will represent current-to-old mappings. */
  INTV_T e_to_eold_map(ell, 0);
  for (int e : mesh.edges.all_indices())
    e_to_eold_map[e] = e;

  /* Initialize local storage for new mappings. */
  INTV_T e_to_enew_map(ell, 0);
  INTV_T eg_to_egnew_map(egll, 0);

  { /* Renumber_EMaps[RenumWaveMinMax]-->RenumWaveMinMaxE */
    /* Initialize new indices to current indices. */
    for (int e : mesh.edges.all_indices())
      e_to_enew_map[e] = e;

    Op_t op = MIN;
    do { /* Edges_minmax */
      /* Access database. */
      auto const &edge_type = mesh.edges.mask;
      auto const &e_to_p1_map = mesh.ds->caccess_intv("m:e>p1");
      auto const &e_to_p2_map = mesh.ds->caccess_intv("m:e>p2");

      /* Use simple map for ghosts. */
      for (int eg : mesh.edges.ghost_indices_offset())
        eg_to_egnew_map[eg] = eg;

      /* Create new e->p mappings. */
      INTV_T e_to_p_map_new(ell, 0);

      /* Fill help array based on min/max point number and flag. */
      for (int e : mesh.edges.local_indices()) {
        if (edge_type[e] == 0)
          continue;

        int const enew = e_to_enew_map[e];
        int const p1 = e_to_p1_map[e];
        int const p2 = e_to_p2_map[e];

        if (MIN == op) {
          e_to_p_map_new[enew] = std::min(p1, p2);
        } else if (MAX == op) {
          e_to_p_map_new[enew] = std::max(p1, p2);
        }
      }

      /* Generate the new numbers. */
      int e_max = 0;
      INTV_T enew_to_enew2_map(ell, 0);
      new_numbering(mesh.edges, mesh.points,
                    e_to_p_map_new,
                    e_max, enew_to_enew2_map);

      /* The first iteration translates X->XNEW and the second iteration
       * translates XNEW->XNEW2. The X->XNEW map contains both
       * translations X->XNEW->XNEW2. */
      for (int e : mesh.edges.all_indices()) {
        int const enew = e_to_enew_map[e];
        if (enew == 0)
          continue;

        int const enew2 = enew_to_enew2_map[enew];
        e_to_enew_map[e] = enew2;
      }

      op += 1;
    } while(op <= MAX);
  }

  { /* ReshapeE() */
  }
}

/* Renumber_C[kkcll]-->Renumb_C */
void renumber_c(Mesh &mesh) {
  /* Get sizes for general use. */
  int const cll = mesh.corners.size();
  int const cgl = mesh.corners.ghost_local_size();
  int const cgll = mesh.corners.ghost_size();

  /* Store new-to-old mappings for debugging/testing. After the reshape,
   * these will represent current-to-old mappings. */
  INTV_T c_to_cold_map(cll, 0);
  for (int c : mesh.corners.all_indices())
    c_to_cold_map[c] = c;

  /* Initialize local storage for new mappings. */
  INTV_T c_to_cnew_map(cll, 0);
  INTV_T cg_to_cgnew_map(cgll, 0);

  { /* Renumber_CMaps[RenumWaveMinMax]-->RenumWaveMinMaxC */
    /* Renumber corners based on point number. */
    { /* Corner_minmax */
      /* Access database. */
      auto const &c_to_p_map = mesh.ds->caccess_intv("m:c>p");
      auto const &ghost_corner_type = mesh.corners.ghost_mask;
      auto const &cg_to_c_map = mesh.corners.cpy_idx;

      /* Use simple map for ghosts. */
      for (int cg : mesh.corners.ghost_indices_offset())
        cg_to_cgnew_map[cg] = cg;

      /* Generate the new numbers. */
      int c_max = 0;
      new_numbering(mesh.corners, mesh.points,
                    c_to_p_map,
                    c_max, c_to_cnew_map);

      if (cgl > 0) { // Set new ghost indices, if there are ghosts
        for (int cg : mesh.corners.ghost_indices_offset()) {
          if (ghost_corner_type[cg] == 0)
            continue;

          int const c = cg_to_c_map[cg];
          c_max += 1;
          c_to_cnew_map[c] = c_max;
        }
      }
    }
  }

  { /* ReshapeC() */
  }
}

} // namespace Ume
