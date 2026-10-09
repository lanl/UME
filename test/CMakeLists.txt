# Ume/test/CMakeLists.txt

# test/ links the targets src/ defines and may gate on the variables the
# pkgs/ modules set.  Added any earlier, it sees neither on a fresh configure
# and reads the variables back from the cache on a reconfigure.
if(NOT TARGET Ume)
  message(FATAL_ERROR
    "test/ must be added after src/, from the top-level CMakeLists.txt")
endif()

# UME standard tests

add_executable(ume_tests
  test_comm_buffers.cc
  test_datastore.cc
  test_entity_comm.cc
  test_io.cc
  test_utils.cc
  test_vecn.cc
)

target_link_libraries(ume_tests PRIVATE
  Ume Catch2::Catch2WithMain)

# HAVE_MPI, among others: test_comm_buffers.cc exchanges through Comm::MPI when
# the library was built with it.
target_compile_definitions(ume_tests PRIVATE ${COMMON_COMPILE_DEFINITIONS})

# The project does not include(CTest), so there is no default timeout: without
# one, a case that hangs, or that allocates without bound on a corrupt length,
# stalls the run instead of failing.
catch_discover_tests(ume_tests PROPERTIES TIMEOUT 120)

# UME GPU-enabled tests

# Note: UME_SERIAL is defined in the top-level CMakeLists.txt.

add_executable(ume_gpu_tests
  test_atomic_add.cc
  test_mesh.cc
  test_scratch_arrays.cc
  custom_main.cc
)

target_link_libraries(ume_gpu_tests PRIVATE
  Ume Catch2::Catch2)

# The same bound as ume_tests'.
catch_discover_tests(ume_gpu_tests PROPERTIES TIMEOUT 120)

# UME build-configuration tests

# Every translation unit has to agree on UME_SERIAL; check_ume_serial.cmake
# reads the compilation database, which only the Ninja and Makefile generators
# write.
if(CMAKE_EXPORT_COMPILE_COMMANDS AND CMAKE_GENERATOR MATCHES "Ninja|Makefiles")
  add_test(NAME cmake_ume_serial_consistent
    COMMAND ${CMAKE_COMMAND}
      -DCOMPILE_COMMANDS=${CMAKE_BINARY_DIR}/compile_commands.json
      -DUME_SERIAL=${UME_SERIAL}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/check_ume_serial.cmake
    )
endif()

# error_stop tests

# Pool exhaustion has to print its message and exit with a nonzero status.
# The exhaustion leaves claims outstanding and the pool's Finalize asserts that
# none are, so a Debug build dies on that assertion if anything on the error
# path finalizes the pool: error_stop calling finalize() (once before it
# printed, losing the message, then after), or a static destructor that
# std::exit runs.  With a 2 MB pool the same probe's claims fit, and the
# message must not appear.
add_executable(error_stop_probe error_stop_probe.cc)
target_link_libraries(error_stop_probe PRIVATE Ume)
target_compile_definitions(error_stop_probe PRIVATE
  ${COMMON_COMPILE_DEFINITIONS})
set(_error_stop_check
  -DPROBE=$<TARGET_FILE:error_stop_probe>
  -P ${CMAKE_CURRENT_SOURCE_DIR}/check_error_stop.cmake)
add_test(NAME error_stop_pool_exhausted
  COMMAND ${CMAKE_COMMAND}
    -DPOOL_MB=1
    "-DEXPECT=Memory pool cannot claim memory\\."
    "-DREJECT=both claims fit"
    ${_error_stop_check})
add_test(NAME error_stop_pool_fits
  COMMAND ${CMAKE_COMMAND}
    -DPOOL_MB=2
    -DEXPECT_SUCCESS=ON
    "-DEXPECT=error_stop_probe: both claims fit"
    "-DREJECT=Memory pool cannot claim memory"
    ${_error_stop_check})
set_tests_properties(error_stop_pool_exhausted error_stop_pool_fits
  PROPERTIES TIMEOUT 90)

# Driver test meshes

# Writes a one-partition mesh holding only zones with the given masks; the
# scale_mesh and ume_mpi driver tests below read it.
add_executable(write_zone_mask_mesh write_zone_mask_mesh.cc)
target_link_libraries(write_zone_mask_mesh PRIVATE Ume)
target_compile_definitions(write_zone_mask_mesh PRIVATE
  ${COMMON_COMPILE_DEFINITIONS})

# Runs a driver through check_exit_status.cmake, which holds it to an exit
# status as well as to its output.
set(_exit_status_check
  -P ${CMAKE_CURRENT_SOURCE_DIR}/check_exit_status.cmake)

# scale_mesh driver tests

# Each failure below has to exit 1 as well as say why, so each runs through
# check_exit_status.cmake: under a pass expression ctest ignores the exit
# status, and a scale_mesh that printed the message and exited 0 passed.

# A basename with no partition file behind it has to stop scale_mesh before it
# reads the unopened stream as a mesh.  "Aborting." is printed only on the
# failure path; an unchecked open went on to Mesh::read, which in a Debug
# build stops at an assertion instead.
add_test(NAME scale_mesh_unreadable_mesh
  COMMAND ${CMAKE_COMMAND}
    -DPROGRAM=$<TARGET_FILE:scale_mesh>
    "-DARGS=${CMAKE_CURRENT_BINARY_DIR}/no-such-mesh$<SEMICOLON>2"
    -DSTATUS=1
    "-DEXPECT=Unable to open file \"[^\"]*/no-such-mesh\\.00000\\.ume\" for reading\\.[\r\n]+Aborting\\."
    ${_exit_status_check})
set_tests_properties(scale_mesh_unreadable_mesh PROPERTIES TIMEOUT 90)

# An output file that cannot be opened has to fail the run too: an unchecked
# open wrote nothing, printed "Done." and exited 0.  A directory sits where
# scale 1 writes its partition file.
set(_scale_mesh_dir ${CMAKE_CURRENT_BINARY_DIR}/scale_mesh_meshes)
file(MAKE_DIRECTORY ${_scale_mesh_dir}/unwritable.00001.00000.ume)
add_test(NAME scale_mesh_write_input_mesh
  COMMAND write_zone_mask_mesh ${_scale_mesh_dir}/unwritable.00000.ume 1)
set_tests_properties(scale_mesh_write_input_mesh PROPERTIES
  FIXTURES_SETUP scale_mesh_input_mesh)
add_test(NAME scale_mesh_unwritable_output
  COMMAND ${CMAKE_COMMAND}
    -DPROGRAM=$<TARGET_FILE:scale_mesh>
    "-DARGS=${_scale_mesh_dir}/unwritable$<SEMICOLON>1"
    -DSTATUS=1
    "-DEXPECT=Unable to open file \"[^\"]*/unwritable\\.00001\\.00000\\.ume\" for writing\\.[\r\n]+Aborting\\."
    ${_exit_status_check})
set_tests_properties(scale_mesh_unwritable_output PROPERTIES
  FIXTURES_REQUIRED scale_mesh_input_mesh
  TIMEOUT 90)

# A partition file that ends early has to fail the run too: Mesh::read goes
# quiet once its stream fails, and scale_mesh scaled and wrote what it had and
# printed "Done.".  Ten bytes off the end take the zones' subset count and the
# newlines after it; ume_serial_truncated_midway_mesh cuts ahead of later tags.
# The same mesh written whole reaches "Done.", the control.
add_test(NAME scale_mesh_write_truncated_mesh
  COMMAND write_zone_mask_mesh --drop-last=10
    ${_scale_mesh_dir}/truncated.00000.ume 1 1 -1 -1)
add_test(NAME scale_mesh_write_intact_mesh
  COMMAND write_zone_mask_mesh ${_scale_mesh_dir}/intact.00000.ume 1 1 -1 -1)
set_tests_properties(scale_mesh_write_truncated_mesh PROPERTIES
  FIXTURES_SETUP scale_mesh_truncated_input)
set_tests_properties(scale_mesh_write_intact_mesh PROPERTIES
  FIXTURES_SETUP scale_mesh_intact_input)
add_test(NAME scale_mesh_truncated_mesh
  COMMAND ${CMAKE_COMMAND}
    -DPROGRAM=$<TARGET_FILE:scale_mesh>
    "-DARGS=${_scale_mesh_dir}/truncated$<SEMICOLON>1"
    -DSTATUS=1
    "-DEXPECT=Unable to read a mesh from file \"[^\"]*/truncated\\.00000\\.ume\"\\.[\r\n]+Aborting\\."
    ${_exit_status_check})
set_tests_properties(scale_mesh_truncated_mesh PROPERTIES
  FIXTURES_REQUIRED scale_mesh_truncated_input
  TIMEOUT 90)
add_test(NAME scale_mesh_intact_mesh
  COMMAND scale_mesh ${_scale_mesh_dir}/intact 1)
set_tests_properties(scale_mesh_intact_mesh PROPERTIES
  FIXTURES_REQUIRED scale_mesh_intact_input
  PASS_REGULAR_EXPRESSION "Done\\."
  FAIL_REGULAR_EXPRESSION "Aborting\\."
  TIMEOUT 90)

# An output file that opens but cannot be written, as on a full disk, has to
# fail the run as well: an unchecked write left an empty file, printed "Done."
# and exited 0.  A file-size limit of zero stands in for the full disk, with
# SIGXFSZ ignored so that the write fails with EFBIG instead of killing the
# process; the limit does not apply to the read.  scale_mesh_intact_mesh, the
# same mesh with no limit, is the control.  Registered only without MPI: an MPI
# build's scale_mesh starts as an OpenMPI singleton, and on Linux its PMIx
# startup cannot create its shared-memory segment under the zero limit
# (PMIX_ERR_OUT_OF_RESOURCE), so MPI_Init aborts before the write is tried.
# write_mesh's stream check has no MPI branch, and the non-MPI builds run it.
# The check runs sh, which sets both and execs scale_mesh, for the reason
# txt2bin_output_over_size_limit gives.
if(UNIX AND NOT USE_MPI)
  add_test(NAME scale_mesh_write_size_limited_mesh
    COMMAND write_zone_mask_mesh
      ${_scale_mesh_dir}/size_limited.00000.ume 1 1 -1 -1)
  set_tests_properties(scale_mesh_write_size_limited_mesh PROPERTIES
    FIXTURES_SETUP scale_mesh_size_limited_input)
  set(_scale_mesh_size_limited_args
    -c "trap '' XFSZ && ulimit -f 0 && exec \"$0\" \"$@\""
    $<TARGET_FILE:scale_mesh> ${_scale_mesh_dir}/size_limited 1)
  list(JOIN _scale_mesh_size_limited_args "$<SEMICOLON>"
    _scale_mesh_size_limited_args)
  add_test(NAME scale_mesh_output_over_size_limit
    COMMAND ${CMAKE_COMMAND}
      -DPROGRAM=sh
      "-DARGS=${_scale_mesh_size_limited_args}"
      -DSTATUS=1
      "-DEXPECT=Unable to write a mesh to file \"[^\"]*/size_limited\\.00001\\.00000\\.ume\"\\.[\r\n]+Aborting\\."
      ${_exit_status_check})
  set_tests_properties(scale_mesh_output_over_size_limit PROPERTIES
    FIXTURES_REQUIRED scale_mesh_size_limited_input
    TIMEOUT 90)
endif()

# Writes a one-partition mesh of points, or prints the points of one; the
# stitch tests below use it on both sides of scale_mesh.
add_executable(point_mesh point_mesh.cc)
target_link_libraries(point_mesh PRIVATE Ume)
target_compile_definitions(point_mesh PRIVATE ${COMMON_COMPILE_DEFINITIONS})

# Scale 2 stitches one copy of the mesh to its +x side, translated by the
# mesh's x extent.  The extent was taken as the largest x, which is right
# only when the smallest is 0: on [-1, 0] the copy landed on its original, and
# on [1, 3] it left a gap of 1.  Each test writes the mesh on the given
# coordinates, scales it, and matches every point of the result.  A copy
# keeps its original's y and z: scale_mesh once set only the stitched
# component of a copied point and left the other two at whatever the resize
# put there, which a mesh with every point at y = z = 0 could not show and
# the off_axis mesh does.  The file the read-back reads is removed before the
# stitch runs: left from an earlier run, it let the read-back pass on a
# scale_mesh that wrote its output under another name.
function(add_scale_mesh_stitch_test name expected)
  set(_basename ${_scale_mesh_dir}/${name})
  set(_output ${_basename}.00002.00000.ume)
  add_test(NAME scale_mesh_write_${name}_mesh
    COMMAND point_mesh write ${_basename}.00000.ume ${ARGN})
  add_test(NAME scale_mesh_remove_${name}_output
    COMMAND ${CMAKE_COMMAND} -E rm -f ${_output})
  set_tests_properties(
    scale_mesh_write_${name}_mesh scale_mesh_remove_${name}_output PROPERTIES
    FIXTURES_SETUP scale_mesh_${name}_input)
  add_test(NAME scale_mesh_stitch_${name}
    COMMAND scale_mesh ${_basename} 2)
  set_tests_properties(scale_mesh_stitch_${name} PROPERTIES
    FIXTURES_REQUIRED scale_mesh_${name}_input
    FIXTURES_SETUP scale_mesh_${name}_output
    TIMEOUT 90)
  add_test(NAME scale_mesh_stitched_${name}_points
    COMMAND point_mesh print ${_output})
  set_tests_properties(scale_mesh_stitched_${name}_points PROPERTIES
    FIXTURES_REQUIRED scale_mesh_${name}_output
    PASS_REGULAR_EXPRESSION "^${expected}$")
endfunction()

set(_p "[\r\n]+point")
add_scale_mesh_stitch_test(negative
  "point 0: -1 0 0${_p} 1: 0 0 0${_p} 2: 0 0 0${_p} 3: 1 0 0[\r\n]*"
  -1 0)
add_scale_mesh_stitch_test(positive
  "point 0: 1 0 0${_p} 1: 3 0 0${_p} 2: 3 0 0${_p} 3: 5 0 0[\r\n]*"
  1 3)
add_scale_mesh_stitch_test(off_axis
  "point 0: 0 1 2${_p} 1: 1 -3 0.5${_p} 2: 1 1 2${_p} 3: 2 -3 0.5[\r\n]*"
  0,1,2 1,-3,0.5)

# Writes a one-partition mesh with every entity, or prints the entity data of
# one; the tests below use it on both sides of scale_mesh.
add_executable(entity_mesh entity_mesh.cc)
target_link_libraries(entity_mesh PRIVATE Ume)
target_compile_definitions(entity_mesh PRIVATE ${COMMON_COMPILE_DEFINITIONS})

# The stitched copy of each entity has to carry its original's mask,
# communication type and subsets, and its index maps have to keep the -1
# sentinels.  scale_mesh copied only the index maps and the coordinates: the
# copied half had mask 0, which every kernel skips, and no subsets, and the
# offset turned each sentinel into the original's last entity: a boundary
# face's missing second zone, `m:f>z2 == -1`, became a zone of the original.
# The expected lines are entity_mesh's mesh doubled along x by hand.  The
# output is removed before the stitch, as for the point meshes above.
set(_entity_lines
  "points lsize 4 size 4"
  "points mask: 1 -1 1 -1"
  "points comm_type: 1 2 1 2"
  "points subset corner lsize 2 elements: 1 3 mask: 3 3"
  "edges lsize 2 size 2"
  "edges mask: 1 1"
  "edges comm_type: 1 1"
  "faces lsize 6 size 6"
  "faces mask: 1 -1 1 1 -1 1"
  "faces comm_type: 1 2 3 1 2 3"
  "faces subset boundary lsize 4 elements: 1 2 4 5 mask: -1 -2 -1 -2"
  "sides lsize 4 size 4"
  "sides mask: 1 -1 1 -1"
  "sides comm_type: 1 2 1 2"
  "corners lsize 4 size 4"
  "corners mask: 1 -1 1 -1"
  "corners comm_type: 1 2 1 2"
  "zones lsize 4 size 4"
  "zones mask: 1 -1 1 -1"
  "zones comm_type: 1 2 1 2"
  "zones subset inner lsize 2 elements: 0 2 mask: 1 1"
  "zones subset all lsize 4 elements: 0 1 2 3 mask: 1 -1 1 -1"
  "iotas lsize 2 size 2"
  "iotas mask: 1 1"
  "iotas comm_type: 1 1"
  "m:e>p1: 0 2"
  "m:e>p2: 1 3"
  "m:f>z1: 0 0 1 2 2 3"
  "m:f>z2: 1 -1 -1 3 -1 -1"
  "m:s>z: 0 1 2 3"
  "m:s>p1: 0 1 2 3"
  "m:s>p2: 1 0 3 2"
  "m:s>e: 0 0 1 1"
  "m:s>f: 0 2 3 5"
  "m:s>c1: 0 1 2 3"
  "m:s>c2: 1 0 3 2"
  "m:s>s2: 1 0 3 2"
  "m:s>s3: 0 1 2 3"
  "m:s>s4: 1 0 3 2"
  "m:s>s5: 1 -1 3 -1"
  "m:c>p: 0 1 2 3"
  "m:c>z: 0 1 2 3"
  "m:a>z: 1 3"
  "m:a>f: 2 5"
  "m:a>p: 1 3"
  "m:a>e: 0 1"
  "m:a>s: 1 3")
string(JOIN "[\r\n]+" _entity_expected ${_entity_lines})
set(_entity_output ${_scale_mesh_dir}/entities.00002.00000.ume)
add_test(NAME scale_mesh_write_entity_mesh
  COMMAND entity_mesh write ${_scale_mesh_dir}/entities.00000.ume)
add_test(NAME scale_mesh_remove_entity_output
  COMMAND ${CMAKE_COMMAND} -E rm -f ${_entity_output})
set_tests_properties(
  scale_mesh_write_entity_mesh scale_mesh_remove_entity_output PROPERTIES
  FIXTURES_SETUP scale_mesh_entity_input)
add_test(NAME scale_mesh_stitch_entities
  COMMAND scale_mesh ${_scale_mesh_dir}/entities 2)
set_tests_properties(scale_mesh_stitch_entities PROPERTIES
  FIXTURES_REQUIRED scale_mesh_entity_input
  FIXTURES_SETUP scale_mesh_entity_output
  PASS_REGULAR_EXPRESSION "Done\\."
  FAIL_REGULAR_EXPRESSION "Aborting\\."
  TIMEOUT 90)
add_test(NAME scale_mesh_stitched_entities
  COMMAND entity_mesh print ${_entity_output})
set_tests_properties(scale_mesh_stitched_entities PROPERTIES
  FIXTURES_REQUIRED scale_mesh_entity_output
  PASS_REGULAR_EXPRESSION "^${_entity_expected}[\r\n]*$")

# scale_mesh stitches a single-rank mesh without ghosts and has to refuse any
# other: a partition of a larger mesh, or a mesh with ghosts, whose copies
# would land among the local entities.  It went on and wrote a mesh whose
# copied half had no exchange lists.  scale_mesh_stitch_entities, the same
# mesh with one rank and no ghosts, is the control.
foreach(_refused IN ITEMS numpe ghost)
  if(_refused STREQUAL "numpe")
    set(_flag --numpe=2)
    set(_message "scale_mesh needs a single-rank mesh\\; this is rank 0 of 2")
  else()
    set(_flag --ghost-zone)
    set(_message "scale_mesh needs a mesh without ghosts\\; its zones \\[2, 3\\) are ghosts")
  endif()
  add_test(NAME scale_mesh_write_${_refused}_mesh
    COMMAND entity_mesh write ${_scale_mesh_dir}/${_refused}.00000.ume ${_flag})
  set_tests_properties(scale_mesh_write_${_refused}_mesh PROPERTIES
    FIXTURES_SETUP scale_mesh_${_refused}_input)
  add_test(NAME scale_mesh_refuses_${_refused}
    COMMAND ${CMAKE_COMMAND}
      -DPROGRAM=$<TARGET_FILE:scale_mesh>
      "-DARGS=${_scale_mesh_dir}/${_refused}$<SEMICOLON>2"
      -DSTATUS=1
      "-DEXPECT=${_message}\\.[\r\n]+Aborting\\."
      "-DREJECT=Done\\."
      ${_exit_status_check})
  set_tests_properties(scale_mesh_refuses_${_refused} PROPERTIES
    FIXTURES_REQUIRED scale_mesh_${_refused}_input
    TIMEOUT 90)
endforeach()

# The scale factor has to be a whole positive power of 2.  atoi read "1x" as
# 1 and scaled by it; it read the most negative int as itself, which the
# power-of-2 test `scale & (scale - 1)` passed (by signed overflow), and the
# run wrote an unscaled mesh under a negative scale.  scale_mesh checks the
# scale before it reads the mesh, so these need none written.
# scale_mesh_intact_mesh, scale 1 on the same basename, is the control.
foreach(_bad_scale IN ITEMS 1x -2147483648)
  add_test(NAME scale_mesh_bad_scale_${_bad_scale}
    COMMAND ${CMAKE_COMMAND}
      -DPROGRAM=$<TARGET_FILE:scale_mesh>
      "-DARGS=${_scale_mesh_dir}/intact$<SEMICOLON>${_bad_scale}"
      -DSTATUS=1
      "-DEXPECT=Scale must be a power of 2, got \"${_bad_scale}\""
      "-DREJECT=Done\\."
      ${_exit_status_check})
  set_tests_properties(scale_mesh_bad_scale_${_bad_scale} PROPERTIES
    TIMEOUT 90)
endforeach()

# The scale factor has to be checked before the mesh is read, as ume_mpi
# checks its arguments: scale_mesh read the whole mesh first, so a bad scale
# was reported only after a read that can take minutes, and not at all when
# the read failed.  The basename is scale_mesh_unreadable_mesh's, which with
# scale 2 is the control: the same missing file reported as "Unable to open".
add_test(NAME scale_mesh_bad_scale_before_read
  COMMAND ${CMAKE_COMMAND}
    -DPROGRAM=$<TARGET_FILE:scale_mesh>
    "-DARGS=${CMAKE_CURRENT_BINARY_DIR}/no-such-mesh$<SEMICOLON>3"
    -DSTATUS=1
    "-DEXPECT=Scale must be a power of 2, got \"3\""
    "-DREJECT=Unable to open"
    ${_exit_status_check})
set_tests_properties(scale_mesh_bad_scale_before_read PROPERTIES TIMEOUT 90)

# ume_serial driver tests

# A rank file that ends early has to fail the run as one that cannot be
# opened does: ume_serial checked only the open, read the file as far as it
# went and computed on it, and exited 0.  The cut is scale_mesh_truncated_mesh's.
# The same mesh whole exits 0, the control.
set(_ume_serial_dir ${CMAKE_CURRENT_BINARY_DIR}/ume_serial_meshes)
file(MAKE_DIRECTORY ${_ume_serial_dir})
add_test(NAME ume_serial_write_truncated_mesh
  COMMAND write_zone_mask_mesh --drop-last=10
    ${_ume_serial_dir}/truncated.00000.ume 1 1 -1 -1)
add_test(NAME ume_serial_write_intact_mesh
  COMMAND write_zone_mask_mesh ${_ume_serial_dir}/intact.00000.ume 1 1 -1 -1)
# The same mesh cut in half, 502 of its 1004 bytes off the end: the file ends
# inside the length word of the sides' cpy_idx, ahead of the sides' neighbor
# tags and the corners' and zones' tags.  Each of those was asserted on even
# once the stream had failed, so a Debug build died on SIGABRT before the read
# could be reported.
add_test(NAME ume_serial_write_truncated_midway_mesh
  COMMAND write_zone_mask_mesh --drop-last=502
    ${_ume_serial_dir}/truncated-midway.00000.ume 1 1 -1 -1)
set_tests_properties(
  ume_serial_write_truncated_mesh ume_serial_write_intact_mesh
  ume_serial_write_truncated_midway_mesh PROPERTIES
  FIXTURES_SETUP ume_serial_meshes)
add_test(NAME ume_serial_truncated_mesh
  COMMAND ${CMAKE_COMMAND}
    -DPROGRAM=$<TARGET_FILE:ume_serial>
    -DARGS=${_ume_serial_dir}/truncated.00000.ume
    -DSTATUS=1
    "-DEXPECT=Unable to read a mesh from file \"[^\"]*/truncated\\.00000\\.ume\"\\."
    ${_exit_status_check})
add_test(NAME ume_serial_intact_mesh
  COMMAND ${CMAKE_COMMAND}
    -DPROGRAM=$<TARGET_FILE:ume_serial>
    -DARGS=${_ume_serial_dir}/intact.00000.ume
    -DSTATUS=0
    "-DEXPECT=Reading: [^\r\n]*/intact\\.00000\\.ume"
    "-DREJECT=Unable to"
    ${_exit_status_check})
add_test(NAME ume_serial_truncated_midway_mesh
  COMMAND ${CMAKE_COMMAND}
    -DPROGRAM=$<TARGET_FILE:ume_serial>
    -DARGS=${_ume_serial_dir}/truncated-midway.00000.ume
    -DSTATUS=1
    "-DEXPECT=Unable to read a mesh from file \"[^\"]*/truncated-midway\\.00000\\.ume\"\\."
    ${_exit_status_check})
set_tests_properties(ume_serial_truncated_mesh ume_serial_intact_mesh
  ume_serial_truncated_midway_mesh
  PROPERTIES
  FIXTURES_REQUIRED ume_serial_meshes
  TIMEOUT 90)

# txt2bin driver tests

# txt2bin_point_mesh.txt is a one-point mesh in the text format.  A binary
# file that opens but cannot be written has to fail the conversion: an
# unchecked write left an empty file, which the verification read then took
# for a mesh (and a Debug build stopped at an assertion in it).  The zero
# file-size limit, with SIGXFSZ ignored, is scale_mesh_output_over_size_limit's.
# The check runs sh, which sets both and execs txt2bin: set on the cmake that
# runs the check instead, the limit would reach txt2bin but the ignored
# SIGXFSZ would not, since execute_process resets the child's signals.
# txt2bin_point_mesh, the same conversion with no limit, is the control.
set(_txt2bin_dir ${CMAKE_CURRENT_BINARY_DIR}/txt2bin_meshes)
file(MAKE_DIRECTORY ${_txt2bin_dir})
set(_txt2bin_input ${CMAKE_CURRENT_SOURCE_DIR}/txt2bin_point_mesh.txt)
add_test(NAME txt2bin_point_mesh
  COMMAND ${CMAKE_COMMAND}
    -DPROGRAM=$<TARGET_FILE:txt2bin>
    "-DARGS=${_txt2bin_input}$<SEMICOLON>${_txt2bin_dir}/point.ume"
    -DSTATUS=0
    "-DEXPECT=Copy verified"
    "-DREJECT=Unable to"
    ${_exit_status_check})
set_tests_properties(txt2bin_point_mesh PROPERTIES TIMEOUT 90)

# Two damaged copies of the same mesh, which txt2bin has to reject with a
# message naming what it found.  truncated.txt stops after the second tag: the
# tag reader took the trailing ':' and ' ' off an empty tag with back(), which
# a Debug build stopped at as an assertion.  long_line.txt has a "Ghost Points"
# header line 300 characters too long: the line reader read into a 256-byte
# buffer, so its message showed the first 255 and lost the rest, here "END".
# txt2bin_point_mesh, the undamaged file, is the control for both.
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  ${_txt2bin_input})
file(STRINGS ${_txt2bin_input} _txt2bin_head LIMIT_COUNT 2)
list(JOIN _txt2bin_head "\n" _txt2bin_head)
file(WRITE ${_txt2bin_dir}/truncated.txt "${_txt2bin_head}\n")
file(READ ${_txt2bin_input} _txt2bin_text)
string(REPEAT "0" 300 _txt2bin_pad)
string(REPLACE "\nGhost Points\n" "\nGhost Points${_txt2bin_pad}END\n"
  _txt2bin_long "${_txt2bin_text}")
file(WRITE ${_txt2bin_dir}/long_line.txt "${_txt2bin_long}")
add_test(NAME txt2bin_truncated_input
  COMMAND ${CMAKE_COMMAND}
    -DPROGRAM=$<TARGET_FILE:txt2bin>
    "-DARGS=${_txt2bin_dir}/truncated.txt$<SEMICOLON>${_txt2bin_dir}/truncated.ume"
    -DSTATUS=1
    "-DEXPECT=Expecting tag \"This rank\", got \"\""
    ${_exit_status_check})
add_test(NAME txt2bin_long_line
  COMMAND ${CMAKE_COMMAND}
    -DPROGRAM=$<TARGET_FILE:txt2bin>
    "-DARGS=${_txt2bin_dir}/long_line.txt$<SEMICOLON>${_txt2bin_dir}/long_line.ume"
    -DSTATUS=1
    "-DEXPECT=Expecting line \"Ghost Points\", got \"Ghost Points0+END"
    ${_exit_status_check})
set_tests_properties(txt2bin_truncated_input txt2bin_long_line
  PROPERTIES TIMEOUT 90)
if(UNIX)
  set(_size_limited_args
    -c "trap '' XFSZ && ulimit -f 0 && exec \"$0\" \"$@\""
    $<TARGET_FILE:txt2bin> ${_txt2bin_input}
    ${_txt2bin_dir}/size_limited.ume)
  list(JOIN _size_limited_args "$<SEMICOLON>" _size_limited_args)
  add_test(NAME txt2bin_output_over_size_limit
    COMMAND ${CMAKE_COMMAND}
      -DPROGRAM=sh
      "-DARGS=${_size_limited_args}"
      -DSTATUS=4
      "-DEXPECT=Unable to write a mesh to file \"[^\"]*/size_limited\\.ume\"\\."
      ${_exit_status_check})
  set_tests_properties(txt2bin_output_over_size_limit PROPERTIES TIMEOUT 90)
endif()

# UME driver startup tests

# ume_mpi run as an MPI singleton: a missing basename, one that the
# --kokkos-* argument stripping leaves missing, and a partition file that
# cannot be read all have to end in Comm::MPI::abort.  One rank cannot show
# the others being taken down rather than left in a gathscat; the abort
# message is what tells abort() from a return out of main().
if(TARGET ume_mpi)
  set(_ume_mpi_check
    -DUME_MPI=$<TARGET_FILE:ume_mpi>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/check_ume_mpi_startup.cmake)
  add_test(NAME ume_mpi_no_basename
    COMMAND ${CMAKE_COMMAND}
      "-DEXPECT=Usage: ume_mpi <basename>.*Transport::abort: no mesh basename"
      ${_ume_mpi_check})
  add_test(NAME ume_mpi_only_kokkos_args
    COMMAND ${CMAKE_COMMAND}
      -DARGS=--kokkos-num-threads=1
      "-DEXPECT=Usage: ume_mpi <basename>.*Transport::abort: no mesh basename"
      ${_ume_mpi_check})
  add_test(NAME ume_mpi_unreadable_mesh
    COMMAND ${CMAKE_COMMAND}
      -DARGS=${CMAKE_CURRENT_BINARY_DIR}/no-such-mesh
      "-DEXPECT=Transport::abort: this rank could not read its mesh file"
      ${_ume_mpi_check})
  set_tests_properties(
    ume_mpi_no_basename ume_mpi_only_kokkos_args ume_mpi_unreadable_mesh
    PROPERTIES TIMEOUT 90)

  # ume_mpi seeds its gradient in one interior zone (mask >= 1) of each rank.
  # Searching upward from the middle of the zone range ran off the end when the
  # upper half held none; a rank with no interior zone at all, which an
  # over-decomposed mesh has, has to take the job down.
  set(_zone_mask_dir ${CMAKE_CURRENT_BINARY_DIR}/zone_mask_meshes)
  file(MAKE_DIRECTORY ${_zone_mask_dir})
  # ume_mpi_long_basename names its mesh by an absolute basename more than
  # 112 characters long, whatever the length of the build directory's path:
  # read_mesh() once formatted the file name into a char[80].
  string(REPEAT "/long-basename" 8 _long_subdir)
  set(_long_basename_dir ${_zone_mask_dir}${_long_subdir})
  file(MAKE_DIRECTORY ${_long_basename_dir})
  add_test(NAME ume_mpi_write_no_zone_mesh
    COMMAND write_zone_mask_mesh no-zones.00000.ume
    WORKING_DIRECTORY ${_zone_mask_dir})
  add_test(NAME ume_mpi_write_lower_interior_mesh
    COMMAND write_zone_mask_mesh lower-interior.00000.ume 1 1 -1 -1
    WORKING_DIRECTORY ${_zone_mask_dir})
  add_test(NAME ume_mpi_write_long_basename_mesh
    COMMAND write_zone_mask_mesh
      ${_long_basename_dir}/lower-interior.00000.ume 1 1 -1 -1)
  # The lower-interior mesh cut short as in scale_mesh_truncated_mesh above.
  add_test(NAME ume_mpi_write_truncated_mesh
    COMMAND write_zone_mask_mesh --drop-last=10
      truncated.00000.ume 1 1 -1 -1
    WORKING_DIRECTORY ${_zone_mask_dir})
  set_tests_properties(
    ume_mpi_write_no_zone_mesh ume_mpi_write_lower_interior_mesh
    ume_mpi_write_long_basename_mesh ume_mpi_write_truncated_mesh PROPERTIES
    FIXTURES_SETUP ume_mpi_zone_mask_meshes)

  add_test(NAME ume_mpi_no_interior_zone
    COMMAND ${CMAKE_COMMAND}
      -DARGS=no-zones
      "-DEXPECT=Transport::abort: .*ume_mpi cannot run an over-decomposed mesh"
      ${_ume_mpi_check}
    WORKING_DIRECTORY ${_zone_mask_dir})
  add_test(NAME ume_mpi_interior_zones_in_lower_half
    COMMAND ${CMAKE_COMMAND}
      -DARGS=lower-interior
      -DEXPECT_SUCCESS=ON
      "-DEXPECT=Checking gradient result.*Done\\."
      ${_ume_mpi_check}
    WORKING_DIRECTORY ${_zone_mask_dir})
  add_test(NAME ume_mpi_long_basename
    COMMAND ${CMAKE_COMMAND}
      -DARGS=${_long_basename_dir}/lower-interior
      -DEXPECT_SUCCESS=ON
      "-DEXPECT=Checking gradient result.*Done\\."
      ${_ume_mpi_check})
  # A partition file that ends early has to take the job down as one that
  # cannot be opened does; unchecked, the rank ran on whatever Mesh::read had
  # reached.  ume_mpi_interior_zones_in_lower_half, on the same mesh whole, is
  # the control.
  add_test(NAME ume_mpi_truncated_mesh
    COMMAND ${CMAKE_COMMAND}
      -DARGS=truncated
      "-DEXPECT=Unable to read a mesh from file \"truncated\\.00000\\.ume\"\\..*Transport::abort: this rank could not read its mesh file"
      ${_ume_mpi_check}
    WORKING_DIRECTORY ${_zone_mask_dir})
  # The count after -i has to be a non-negative integer.  atoi read "-1" as -1,
  # which became SIZE_MAX iterations of every timed loop, and "2x" as 2; both
  # have to take the job down instead.  A plain "2" runs to the end, the
  # control.
  add_test(NAME ume_mpi_negative_iteration_count
    COMMAND ${CMAKE_COMMAND}
      "-DARGS=lower-interior$<SEMICOLON>-i$<SEMICOLON>-1"
      "-DEXPECT=iteration count after -i .*got \"-1\".*Transport::abort: invalid iteration count"
      ${_ume_mpi_check}
    WORKING_DIRECTORY ${_zone_mask_dir})
  add_test(NAME ume_mpi_trailing_garbage_iteration_count
    COMMAND ${CMAKE_COMMAND}
      "-DARGS=lower-interior$<SEMICOLON>-i$<SEMICOLON>2x"
      "-DEXPECT=iteration count after -i .*got \"2x\".*Transport::abort: invalid iteration count"
      ${_ume_mpi_check}
    WORKING_DIRECTORY ${_zone_mask_dir})
  add_test(NAME ume_mpi_iteration_count
    COMMAND ${CMAKE_COMMAND}
      "-DARGS=lower-interior$<SEMICOLON>-i$<SEMICOLON>2"
      -DEXPECT_SUCCESS=ON
      "-DEXPECT=Checking gradient result.*Done\\."
      ${_ume_mpi_check}
    WORKING_DIRECTORY ${_zone_mask_dir})
  # Everything after the basename has to be "-i <count>".  An -i with no count
  # and an argument other than -i ran with one iteration, and anything after
  # the count was ignored; all exited 0, and each has to take the job down with
  # the usage line.
  # ume_mpi_iteration_count above is the control.
  add_test(NAME ume_mpi_missing_iteration_count
    COMMAND ${CMAKE_COMMAND}
      "-DARGS=lower-interior$<SEMICOLON>-i"
      "-DEXPECT=-i needs an iteration count.*Usage: ume_mpi <basename>.*Transport::abort: invalid arguments"
      ${_ume_mpi_check}
    WORKING_DIRECTORY ${_zone_mask_dir})
  add_test(NAME ume_mpi_unknown_argument
    COMMAND ${CMAKE_COMMAND}
      "-DARGS=lower-interior$<SEMICOLON>-n$<SEMICOLON>2"
      "-DEXPECT=unexpected argument \"-n\".*Usage: ume_mpi <basename>.*Transport::abort: invalid arguments"
      ${_ume_mpi_check}
    WORKING_DIRECTORY ${_zone_mask_dir})
  add_test(NAME ume_mpi_argument_after_iteration_count
    COMMAND ${CMAKE_COMMAND}
      "-DARGS=lower-interior$<SEMICOLON>-i$<SEMICOLON>2$<SEMICOLON>extra"
      "-DEXPECT=unexpected argument \"extra\".*Usage: ume_mpi <basename>.*Transport::abort: invalid arguments"
      ${_ume_mpi_check}
    WORKING_DIRECTORY ${_zone_mask_dir})
  set_tests_properties(
    ume_mpi_no_interior_zone ume_mpi_interior_zones_in_lower_half
    ume_mpi_long_basename ume_mpi_truncated_mesh
    ume_mpi_negative_iteration_count ume_mpi_trailing_garbage_iteration_count
    ume_mpi_iteration_count ume_mpi_missing_iteration_count
    ume_mpi_unknown_argument ume_mpi_argument_after_iteration_count PROPERTIES
    FIXTURES_REQUIRED ume_mpi_zone_mask_meshes
    TIMEOUT 90)
endif()

# Multi-rank tests

# Two ranks under the MPI launcher, for what a singleton cannot show: the
# virtual-rank map gathered from every rank, and one rank's Comm::MPI::abort
# taking the other down.  Opt-in, since the launcher's ranks talk over a local
# socket, which a sandboxed run may not be allowed to bind.
option(UME_MULTIRANK_TESTS
  "Register the ctests that run on two ranks under MPIEXEC_EXECUTABLE" OFF)
if(UME_MULTIRANK_TESTS AND TARGET ume_mpi AND MPIEXEC_EXECUTABLE)
  # The gathered virtual-rank map overflowed its array on two or more ranks:
  # each rank sent numpe ints to an array of one per rank.  Catch2 exits
  # nonzero when no case matches the name, so a renamed case fails here.
  add_test(NAME mpi_exchange_two_ranks
    COMMAND ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} 2 ${MPIEXEC_PREFLAGS}
      $<TARGET_FILE:ume_tests>
      "comm MPI: exchange with empty remotes and map virtual ranks")
  set_tests_properties(mpi_exchange_two_ranks PROPERTIES TIMEOUT 120)

  # A rank that cannot read its partition file, missing or cut short, has to
  # take the job down with Comm::MPI::abort; returning from main instead left
  # the other rank waiting.  Rank 0 reads a whole copy of the singleton tests'
  # lower-interior mesh, and rank 1 has no file, or one cut short as in
  # ume_mpi_truncated_mesh.  ume_mpi_interior_zones_in_lower_half, the same
  # mesh whole on one rank, is the control.  MPIEXEC_PREFLAGS reach the
  # launcher; there are no MPIEXEC_POSTFLAGS, since ume_mpi and Catch2 would
  # take them for their own arguments.
  set(_two_rank_dir ${CMAKE_CURRENT_BINARY_DIR}/two_rank_meshes)
  file(MAKE_DIRECTORY ${_two_rank_dir})
  add_test(NAME ume_mpi_write_missing_partition_mesh
    COMMAND write_zone_mask_mesh
      ${_two_rank_dir}/missing.00000.ume 1 1 -1 -1)
  add_test(NAME ume_mpi_write_truncated_partition_mesh_0
    COMMAND write_zone_mask_mesh
      ${_two_rank_dir}/truncated.00000.ume 1 1 -1 -1)
  add_test(NAME ume_mpi_write_truncated_partition_mesh_1
    COMMAND write_zone_mask_mesh --drop-last=10
      ${_two_rank_dir}/truncated.00001.ume 1 1 -1 -1)
  set_tests_properties(
    ume_mpi_write_missing_partition_mesh
    ume_mpi_write_truncated_partition_mesh_0
    ume_mpi_write_truncated_partition_mesh_1 PROPERTIES
    FIXTURES_SETUP ume_mpi_two_rank_meshes)

  set(_two_rank_launcher
    ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} 2 ${MPIEXEC_PREFLAGS})
  list(JOIN _two_rank_launcher "$<SEMICOLON>" _two_rank_launcher)
  add_test(NAME ume_mpi_missing_partition_aborts
    COMMAND ${CMAKE_COMMAND}
      "-DLAUNCHER=${_two_rank_launcher}"
      -DARGS=${_two_rank_dir}/missing
      "-DEXPECT=Unable to open file \"[^\"]*/missing\\.00001\\.ume\" for reading\\..*Transport::abort: this rank could not read its mesh file"
      ${_ume_mpi_check})
  add_test(NAME ume_mpi_truncated_partition_aborts
    COMMAND ${CMAKE_COMMAND}
      "-DLAUNCHER=${_two_rank_launcher}"
      -DARGS=${_two_rank_dir}/truncated
      "-DEXPECT=Unable to read a mesh from file \"[^\"]*/truncated\\.00001\\.ume\"\\..*Transport::abort: this rank could not read its mesh file"
      ${_ume_mpi_check})
  set_tests_properties(
    ume_mpi_missing_partition_aborts ume_mpi_truncated_partition_aborts
    PROPERTIES
    FIXTURES_REQUIRED ume_mpi_two_rank_meshes
    TIMEOUT 90)
elseif(UME_MULTIRANK_TESTS)
  # Asked for and not registered would let a run pass without them.
  message(FATAL_ERROR
    "UME_MULTIRANK_TESTS needs USE_MPI=ON and an MPI launcher; "
    "MPIEXEC_EXECUTABLE is \"${MPIEXEC_EXECUTABLE}\"")
endif()
