#pragma once

#include "Ume/SOA_Idx_Mesh.hh"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <random>
#include <ranges>
#include <vector>

namespace Ume {

namespace SOA_Idx {

/* Given integer indices in the range [0,N), this routine produces an old-to-new
   index map where approximately `move_fraction` of the indices in [0, shuffleN)
   have been moved from their original position. move_fraction should be in the
   range [0.0, 1.0], and shuffleN <= N. The indices in [shuffleN, N) don't move:
   they are equal to their index. */
template <class RNG>
std::vector<int> shuffle_indices(
    size_t const N, size_t const shuffleN, float move_fraction, RNG &rng) {

  assert(move_fraction >= 0.0 && move_fraction <= 1.0);
  assert(shuffleN <= N);

  /* Initialize the old-to-new index map, which we will be doing swaps in */
  std::vector<int> old_new_map(N);
  std::iota(old_new_map.begin(), old_new_map.end(), 0);
  if (move_fraction == 0.0)
    return old_new_map;

  size_t const num_move = (move_fraction == 0.0)
      ? shuffleN
      : static_cast<size_t>(
            std::nearbyint(static_cast<float>(shuffleN) * move_fraction));

  size_t pairs = num_move / 2 + num_move % 2;

  size_t Npairsrc = shuffleN / 2 + shuffleN % 2;
  if (pairs > Npairsrc)
    pairs = Npairsrc; // Max # of swaps available
  std::vector<int> pairsrc(Npairsrc * 2);
  std::iota(pairsrc.begin(), pairsrc.end(), 0);

  /* If N was odd, we need to replace the last pairsrc entry with something in
     the range [0, suffleN), */
  if (pairsrc.size() > shuffleN) {
    assert(pairsrc.size() == N + 1);
    pairsrc.back() = static_cast<int>(shuffleN / 2);
  }
  std::shuffle(pairsrc.begin(), pairsrc.end(), rng);

  /* For an even shuffleN, every index appears once in random order in
     pairsrc. Taking them pairwise (i*2, i*2+1), i = [0,shuffleN/2), and
     exchanging their positions, means every index moves once to a location
     distinct from its starting position. */

  /* Now build an old-to-new index map, doing the requested number of swaps */
  auto curr_pi = pairsrc.begin();
  for (size_t p = 0; p < pairs; ++p) {
    int i1 = *curr_pi++;
    int i2 = *curr_pi++;
    std::swap(old_new_map[i1], old_new_map[i2]);
  }
  return old_new_map;
}

template <class FT>
void shuffle_field(FT &field, std::vector<int> const &old_new_map) requires
    is_field_type<FT> || std::is_same_v<FT, typename Entity::mask_array> {
  assert(old_new_map.size() == field.size());
  FT new_data(field.size());
  for (size_t i = 0; i < field.size(); ++i) {
    new_data[old_new_map[i]] = field[i];
  }
  field.swap(new_data);
}

template <class Cont>
constexpr void remap(Cont &cont, std::vector<int> const &old_new_map) {
  std::transform(cont.begin(), cont.end(), cont.begin(),
      [&](int old) { return old_new_map[old]; });
}

template <class RNG>
void shuffle_corners(Mesh &mesh, float const move_fraction, RNG &rng) {
  int const num_local = mesh.corners.local_size();

  std::vector<int> old_new_map{
      shuffle_indices(mesh.corners.size(), num_local, move_fraction, rng)};
  assert(static_cast<int>(old_new_map.size()) == mesh.corners.size());

  shuffle_field(mesh.corners.mask, old_new_map);
  shuffle_field(mesh.ds->access_intv("m:c>z"), old_new_map);
  shuffle_field(mesh.ds->access_intv("m:c>p"), old_new_map);

  remap(mesh.ds->access_intv("m:s>c1"), old_new_map);
  remap(mesh.ds->access_intv("m:s>c2"), old_new_map);

  mesh.ds->release("corner_vol");
  mesh.ds->release("corner_csurf");
  mesh.ds->release("m:c>ss");

  auto const &cv = mesh.ds->caccess_dblv("corner_vol");
}

} // namespace SOA_Idx

} // namespace Ume
