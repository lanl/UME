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

#define INVALID_INDEX (-1)
#define START_INDEX 0

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
  renumber_a(mesh);
}

/* Renumber_P[kkpll]-->Renumb_P */
void renumber_p(Mesh &mesh) {
  /* Get sizes for general use. */
  int const pll = mesh.points.size();
  int const pgll = mesh.points.ghost_size();

  /* Store new-to-old mappings for debugging/testing. After the reshape,
   * these will represent current-to-old mappings. */
  INTV_T p_to_pold_map(pll, INVALID_INDEX);
  for (int p : mesh.points.all_indices())
    p_to_pold_map[p] = p;

  /* Initialize local storage for new mappings. */
  INTV_T p_to_pnew_map(pll, INVALID_INDEX);
  INTV_T pg_to_pgnew_map(pgll, INVALID_INDEX);

  { /* Renumber_PMaps[RenumWaveMinMax]-->RenumWaveMinMaxP */
    int const all = mesh.iotas.size();
    auto const &iota_type = mesh.iotas.mask;
    auto const &a_to_p_map = mesh.ds->caccess_intv("m:a>p");

    INTV_T p_to_a1_map(pll, INVALID_INDEX), a1_to_a2_map(all, INVALID_INDEX);

    /* Construct P->A linked list. */
    { /* LinkedList */
      int const al = mesh.iotas.local_size();
      INTV_T last(pll, INVALID_INDEX);
      int prev;

      for (int a = 0; a < al; ++a) { /* Sequential only */
        if (iota_type[a] == 0)
          continue;

        int const p = a_to_p_map[a];
        if (INVALID_INDEX == p_to_a1_map[p]) {
          p_to_a1_map[p] = a;
        } else {
          prev = last[p];
          a1_to_a2_map[prev] = a;
        }
        last[p] = a;
      }
    }

    /* Number of a connected to p for each p */
    INTV_T num_a_to_p(pll, 0);
    for (int a : mesh.iotas.local_indices()) {
      if (iota_type[a] == 0)
        continue;

      int const p = a_to_p_map[a];
      num_a_to_p[p] += 1;
    }

    { /* Wave_main */
      /* Access database */
      auto const &point_type = mesh.points.mask;
      auto const &a_to_s_map = mesh.ds->caccess_intv("m:a>s");
      auto const &s_to_p1_map = mesh.ds->caccess_intv("m:s>p1");
      auto const &s_to_p2_map = mesh.ds->caccess_intv("m:s>p2");
      int const sll = mesh.sides.size();

      /* Use simple map for ghosts. */
      for (int pg : mesh.points.ghost_indices_offset())
        pg_to_pgnew_map[pg] = pg;

      /* Count number of non-null points. We'll use this to determine
       * if all points were considered. */
      int totp = 0;
      for (int p : mesh.points.local_indices()) {
        if (point_type[p] != 0)
          totp += 1;
      }

      /* Find first seed point */
      int pseed;
      { /* Wave_seed */
        pseed = 0;
        int max_s = 0, num_s;

        /* Take as the seed point the inactive point with the maximum
         * number of surrounding sides. */
        for (int p : mesh.points.local_indices()) {
          if (point_type[p] == 0)
            continue;

          num_s = num_a_to_p[p];
          if (num_s > max_s && INVALID_INDEX == p_to_pnew_map[p]) {
            pseed = p;
            max_s = num_s;
          }
        }
      }

      /* Load the first point into the front. */
      INTV_T currfront_p(pll, START_INDEX);
      currfront_p[START_INDEX] = pseed;
      int pnew = INVALID_INDEX;

      /* Loop until wavefront has encountered all points. This will be
       * true when the number of points in the current front is zero. */
      INTV_T newfront_p(pll, START_INDEX), side_tag(sll, 0);
      int num_p_currfront = 1;
      while (num_p_currfront > 0) {
        int num_p_newfront = 0;
        int p_newfront_idx = INVALID_INDEX;
        for (int ip = START_INDEX; ip < num_p_currfront; ++ip) {
          int const p = currfront_p[ip];

          /* Loop over all a connected to p. */
          int a = p_to_a1_map[p];
          while (a != INVALID_INDEX) {
            /* Only treat each side once. */
            int const s = a_to_s_map[a];
            if (side_tag[s] == 1) {
              a = a1_to_a2_map[a];
              continue;
            }

            /* Fetch the other point and add it to the next front if it
             * has not been encountered. */
            int sp = s_to_p1_map[s];
            if (p == sp)
              sp = s_to_p2_map[s];
            if (INVALID_INDEX == p_to_pnew_map[sp]) {
              num_p_newfront += 1;
              p_newfront_idx += 1;
              newfront_p[p_newfront_idx] = sp;
              pnew += 1;
              p_to_pnew_map[sp] = pnew;
            }

            side_tag[s] = 1;
            a = a1_to_a2_map[a];
          }
        }

        /* Rebuild the front for the next pass. */
        for (int ip = 0; ip < num_p_newfront; ++ip)
          currfront_p[ip] = newfront_p[ip];
        num_p_currfront = num_p_newfront;

        /* If there is a slideline or if the sub-domain is
         * discontiguous, an advancing wavefront will be required for
         * each disjoint piece. */
        if (num_p_currfront == 0 && (pnew + 1) != totp) {
          /* Find a new seed point. */
          { /* Wave_seed */
            pseed = 0;
            int max_s = 0, num_s;

            /* Take as the seed point the inactive point with the
             * maximum number of surrounding sides. */
            for (int p : mesh.points.local_indices()) {
              if (point_type[p] == 0)
                continue;

              num_s = num_a_to_p[p];
              if (num_s > max_s && INVALID_INDEX == p_to_pnew_map[p]) {
                pseed = p;
                max_s = num_s;
              }
            }
          }

          /* Rebuild front for next pass if not finished yet. */
          if (pseed != 0) {
            num_p_currfront = 1;
            currfront_p[START_INDEX] = pseed;
            pnew += 1;
            p_to_pnew_map[pseed] = pnew;
          }
        }
      }
    }
  }

  { /* ReshapeP() */
  }
}

/* Renumber_S[kksll]-->Renumb_S */
void renumber_s(Mesh &mesh) {
  /* Get sizes for general use. */
  int const sll = mesh.sides.size();
  int const sgl = mesh.sides.ghost_local_size();
  int const sgll = mesh.sides.ghost_size();

  /* Store new-to-old mappings for debugging/testing. After the reshape,
   * these will represent current-to-old mappings. */
  INTV_T s_to_sold_map(sll, INVALID_INDEX);
  for (int s : mesh.sides.all_indices())
    s_to_sold_map[s] = s;

  /* Initialize local storage for new mappings. */
  INTV_T s_to_snew_map(sll, INVALID_INDEX);
  INTV_T sg_to_sgnew_map(sgll, INVALID_INDEX);

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
  INTV_T z_to_zold_map(zll, INVALID_INDEX);
  for (int z : mesh.zones.all_indices())
    z_to_zold_map[z] = z;

  /* Initialize local storage for new mappings. */
  INTV_T z_to_znew_map(zll, INVALID_INDEX);
  INTV_T zg_to_zgnew_map(zgll, INVALID_INDEX);

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
  int const fgll = mesh.faces.ghost_size();

  /* Store new-to-old mappings for debugging/testing. After the reshape,
   * these will represent current-to-old mappings. */
  INTV_T f_to_fold_map(fll, INVALID_INDEX);
  for (int f : mesh.faces.all_indices())
    f_to_fold_map[f] = f;

  /* Initialize local storage for new mappings. */
  INTV_T f_to_fnew_map(fll, INVALID_INDEX);
  INTV_T fg_to_fgnew_map(fgll, INVALID_INDEX);

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
  int const egll = mesh.edges.ghost_size();

  /* Store new-to-old mappings for debugging/testing. After the reshape,
   * these will represent current-to-old mappings. */
  INTV_T e_to_eold_map(ell, INVALID_INDEX);
  for (int e : mesh.edges.all_indices())
    e_to_eold_map[e] = e;

  /* Initialize local storage for new mappings. */
  INTV_T e_to_enew_map(ell, INVALID_INDEX);
  INTV_T eg_to_egnew_map(egll, INVALID_INDEX);

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
  INTV_T c_to_cold_map(cll, INVALID_INDEX);
  for (int c : mesh.corners.all_indices())
    c_to_cold_map[c] = c;

  /* Initialize local storage for new mappings. */
  INTV_T c_to_cnew_map(cll, INVALID_INDEX);
  INTV_T cg_to_cgnew_map(cgll, INVALID_INDEX);

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

/* Renumber_A[kkall]-->Renumb_A */
void renumber_a(Mesh &mesh) {
  /* Get sizes for general use. */
  int const all = mesh.iotas.size();
  int const agl = mesh.iotas.ghost_local_size();
  int const agll = mesh.iotas.ghost_size();

  /* Store new-to-old mappings for debugging/testing. After the reshape,
   * these will represent current-to-old mappings. */
  INTV_T a_to_aold_map(all, INVALID_INDEX);
  for (int a : mesh.iotas.all_indices())
    a_to_aold_map[a] = a;

  /* Initialize local storage for new mappings. */
  INTV_T a_to_anew_map(all, INVALID_INDEX);
  INTV_T ag_to_agnew_map(agll, INVALID_INDEX);

  { /* Renumber_AMaps[RenumWaveMinMax]-->RenumWaveMinMaxA */
    /* Renumber iotas based on point number. */
    { /* Iota_minmax */
      /* Access database. */
      auto const &a_to_p_map = mesh.ds->caccess_intv("m:a>p");
      auto const &ghost_iota_type = mesh.iotas.ghost_mask;
      auto const &ag_to_a_map = mesh.iotas.cpy_idx;

      /* Use simple map for ghosts. */
      for (int ag : mesh.iotas.ghost_indices_offset())
        ag_to_agnew_map[ag] = ag;

      /* Generate the new numbers. */
      int a_max = 0;
      new_numbering(mesh.iotas, mesh.points,
                    a_to_p_map,
                    a_max, a_to_anew_map);

      if (agl > 0) { // Set new ghost indices, if there are ghosts
        for (int ag : mesh.iotas.ghost_indices_offset()) {
          if (ghost_iota_type[ag] == 0)
            continue;

          int const a = ag_to_a_map[ag];
          a_max += 1;
          a_to_anew_map[a] = a_max;
        }
      }
    }
  }

  { /* ReshapeA() */
  }
}

} // namespace Ume
