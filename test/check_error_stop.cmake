# Ume/test/check_error_stop.cmake
#
#   cmake -DPROBE=<path to error_stop_probe> -DPOOL_MB=<n> -DEXPECT=<regex>
#         [-DREJECT=<regex>] [-DEXPECT_SUCCESS=ON] -P check_error_stop.cmake
#
# Runs PROBE with MEMORY_POOL_SIZE_MB=POOL_MB and requires that it exit with a
# nonzero status, or with EXPECT_SUCCESS that it exit 0; that its combined
# stdout and stderr match EXPECT; and, if REJECT is set, that they do not match
# REJECT.  Death by a signal is a failure either way: error_stop has to leave
# through halt(), which exits with EXIT_FAILURE when MPI is not initialized,
# and not die on the pool's assertion that no claims are outstanding, whether
# before halt() or in a static destructor on the way out.
# A script rather than PASS_REGULAR_EXPRESSION on the probe itself: ctest
# ignores the exit status of a test that sets a pass expression.

cmake_minimum_required(VERSION 3.20)

foreach(var IN ITEMS PROBE POOL_MB EXPECT)
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "check_error_stop.cmake: ${var} is not set")
  endif()
endforeach()

set(ENV{MEMORY_POOL_SIZE_MB} "${POOL_MB}")
execute_process(
  COMMAND "${PROBE}"
  RESULT_VARIABLE status
  OUTPUT_VARIABLE out
  ERROR_VARIABLE err
  TIMEOUT 60
  )

set(run "MEMORY_POOL_SIZE_MB=${POOL_MB} ${PROBE}")
set(log "exit status: ${status}\n--- stdout ---\n${out}\n--- stderr ---\n${err}")

if(EXPECT_SUCCESS)
  if(NOT status STREQUAL "0")
    message(FATAL_ERROR "${run} did not exit 0\n${log}")
  endif()
elseif(NOT status MATCHES "^[1-9][0-9]*$")
  message(FATAL_ERROR "${run} did not exit with a nonzero status\n${log}")
endif()
if(NOT "${out}${err}" MATCHES "${EXPECT}")
  message(FATAL_ERROR "${run} output does not match \"${EXPECT}\"\n${log}")
endif()
if(DEFINED REJECT AND "${out}${err}" MATCHES "${REJECT}")
  message(FATAL_ERROR "${run} output matches \"${REJECT}\"\n${log}")
endif()
message(STATUS "${run} exited as expected (${status})")
