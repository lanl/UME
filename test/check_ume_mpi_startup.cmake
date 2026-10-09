# Ume/test/check_ume_mpi_startup.cmake
#
#   cmake -DUME_MPI=<path to ume_mpi> [-DLAUNCHER=<;-list>]
#         -DARGS=<;-list of arguments> -DEXPECT=<regex> [-DEXPECT_SUCCESS=ON]
#         -P check_ume_mpi_startup.cmake
#
# Runs ume_mpi with ARGS, as an MPI singleton or, with LAUNCHER, under the
# launcher command it names (`mpiexec -n 2`, say), and requires that it fail (a
# nonzero exit status), or with EXPECT_SUCCESS that it exit 0, and that its
# combined stdout and stderr match EXPECT.
# The startup failures in ume_mpi have to go through Comm::MPI::abort, which
# takes every rank down, rather than return from main on the one rank that
# noticed; EXPECT names the message that tells the two apart.

cmake_minimum_required(VERSION 3.20)

foreach(var IN ITEMS UME_MPI EXPECT)
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "check_ume_mpi_startup.cmake: ${var} is not set")
  endif()
endforeach()

execute_process(
  COMMAND ${LAUNCHER} "${UME_MPI}" ${ARGS}
  RESULT_VARIABLE status
  OUTPUT_VARIABLE out
  ERROR_VARIABLE err
  TIMEOUT 60
  )

set(log "exit status: ${status}\n--- stdout ---\n${out}\n--- stderr ---\n${err}")

if(EXPECT_SUCCESS)
  if(NOT status STREQUAL "0")
    message(FATAL_ERROR "ume_mpi ${ARGS} did not exit 0\n${log}")
  endif()
elseif(status STREQUAL "0")
  message(FATAL_ERROR "ume_mpi ${ARGS} exited 0; expected a failure\n${log}")
endif()
if(NOT "${out}${err}" MATCHES "${EXPECT}")
  message(FATAL_ERROR
    "ume_mpi ${ARGS} output does not match \"${EXPECT}\"\n${log}")
endif()
message(STATUS "ume_mpi ${ARGS} exited as expected (${status})")
