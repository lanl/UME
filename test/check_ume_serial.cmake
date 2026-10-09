# Ume/test/check_ume_serial.cmake
#
#   cmake -DCOMPILE_COMMANDS=<compile_commands.json> -DUME_SERIAL=<ON|OFF>
#         -P check_ume_serial.cmake
#
# UME_SERIAL selects the DevExecSpace/HostExecSpace typedefs in
# mem_exec_spaces.hh, so the library, the drivers and the tests have to agree
# on it: a translation unit that disagrees with the rest is an ODR violation.
# This reads the compilation database and fails if any translation unit's
# -DUME_SERIAL disagrees with the option.

cmake_minimum_required(VERSION 3.20)

# Set OUT_VAR to the files in the compilation database JSON whose command
# defines UME_SERIAL when EXPECTED is false, or does not when it is true.
function(ume_serial_mismatches JSON EXPECTED OUT_VAR)
  if(EXPECTED)
    set(want TRUE)
  else()
    set(want FALSE)
  endif()
  set(bad "")
  string(JSON n LENGTH "${JSON}")
  if(n GREATER 0)
    math(EXPR last "${n} - 1")
    foreach(i RANGE ${last})
      string(JSON file GET "${JSON}" ${i} file)
      string(JSON command GET "${JSON}" ${i} command)
      if(command MATCHES "(^| )-DUME_SERIAL( |=|$)")
        set(has TRUE)
      else()
        set(has FALSE)
      endif()
      if(NOT has STREQUAL want)
        list(APPEND bad "${file}")
      endif()
    endforeach()
  endif()
  set(${OUT_VAR} "${bad}" PARENT_SCOPE)
endfunction()

# Positive control: a database with one translation unit on each side of the
# option must report exactly the one that disagrees, either way round.
set(control [=[[
  {"file": "on.cc",  "command": "c++ -DUME_SERIAL -c on.cc"},
  {"file": "off.cc", "command": "c++ -DUME_SERIAL_NOT -c off.cc"}
]]=])
ume_serial_mismatches("${control}" ON control_on)
ume_serial_mismatches("${control}" OFF control_off)
if(NOT control_on STREQUAL "off.cc" OR NOT control_off STREQUAL "on.cc")
  message(FATAL_ERROR
    "check_ume_serial is broken: control gave [${control_on}] for ON and "
    "[${control_off}] for OFF, expected [off.cc] and [on.cc]")
endif()

if(NOT DEFINED COMPILE_COMMANDS OR NOT EXISTS "${COMPILE_COMMANDS}")
  message(FATAL_ERROR "No compilation database at '${COMPILE_COMMANDS}'")
endif()
file(READ "${COMPILE_COMMANDS}" database)
string(JSON entries LENGTH "${database}")
if(entries EQUAL 0)
  message(FATAL_ERROR "${COMPILE_COMMANDS} has no entries")
endif()

ume_serial_mismatches("${database}" "${UME_SERIAL}" mismatches)
if(mismatches)
  list(LENGTH mismatches count)
  list(JOIN mismatches "\n  " listing)
  message(FATAL_ERROR
    "UME_SERIAL=${UME_SERIAL}, but ${count} of ${entries} translation units "
    "disagree:\n  ${listing}")
endif()
message(STATUS
  "UME_SERIAL=${UME_SERIAL}: all ${entries} translation units agree")
