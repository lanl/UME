# Ume/test/check_exit_status.cmake
#
#   cmake -DPROGRAM=<path> [-DARGS=<;-list of arguments>] -DSTATUS=<n>
#         -DEXPECT=<regex> [-DREJECT=<regex>] -P check_exit_status.cmake
#
# Runs PROGRAM with ARGS and requires that it exit with STATUS, that its
# combined stdout and stderr match EXPECT, and, if REJECT is set, that they do
# not match REJECT.
# A script rather than PASS_REGULAR_EXPRESSION on the program itself: ctest
# ignores the exit status of a test that sets a pass expression, and a driver
# that reports a failure has to exit nonzero as well as print it.

cmake_minimum_required(VERSION 3.20)

foreach(var IN ITEMS PROGRAM STATUS EXPECT)
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "check_exit_status.cmake: ${var} is not set")
  endif()
endforeach()

execute_process(
  COMMAND "${PROGRAM}" ${ARGS}
  RESULT_VARIABLE status
  OUTPUT_VARIABLE out
  ERROR_VARIABLE err
  TIMEOUT 60
  )

list(JOIN ARGS " " args)
set(run "${PROGRAM} ${args}")
set(log "exit status: ${status}\n--- stdout ---\n${out}\n--- stderr ---\n${err}")

if(NOT status STREQUAL "${STATUS}")
  message(FATAL_ERROR "${run} did not exit ${STATUS}\n${log}")
endif()
if(NOT "${out}${err}" MATCHES "${EXPECT}")
  message(FATAL_ERROR "${run} output does not match \"${EXPECT}\"\n${log}")
endif()
if(DEFINED REJECT AND "${out}${err}" MATCHES "${REJECT}")
  message(FATAL_ERROR "${run} output matches \"${REJECT}\"\n${log}")
endif()
message(STATUS "${run} exited as expected (${status})")
