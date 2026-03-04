# Look for a file and set VAR to the fully qualified path of it's parent
# directory
function(SimpleIncludeDirs PKG VAR MODULE_DIR HEADERFILE)
  # Look in the module-supplied path first
  find_path(
    ${VAR} ${HEADERFILE}
    PATHS ENV ${MODULE_DIR}
    NO_DEFAULT_PATH
    )
  # If that didn't succeed, try a more general search
  find_path(
    ${VAR} ${HEADERFILE}
    NO_SYSTEM_ENVIRONMENT_PATH
    )
  # Not found
  if(NOT ${VAR})
    set(${PKG}_NOT_FOUND TRUE CACHE BOOL
      "${PKG} is not found.")
    mark_as_advanced(${PKG}_NOT_FOUND)
  endif()
endfunction()


# Look for a (single) library file and set VAR to it's fully qualified path
function(SimpleLibraries PKG VAR MODULE_DIR LIBRARY)
  # Look in the module-supplied path first
  find_library(
    ${VAR} ${LIBRARY}
    PATHS ENV ${MODULE_DIR}
    NO_DEFAULT_PATH
    )
  # If that didn't succeed, try a more general search
  find_library(
    ${VAR} ${LIBRARY}
    NO_SYSTEM_ENVIRONMENT_PATH
    )
  # Not found
  if(NOT ${VAR})
    set(${PKG}_NOT_FOUND TRUE CACHE BOOL
      "${PKG} is not found.")
    mark_as_advanced(${PKG}_NOT_FOUND)
  endif()
endfunction()


macro(SimplePkg PKG HEADERFILE LIBRARY)
  # Guard to see if this has been run already
  if(NOT CHECK_${PKG}_DONE)
    # Look for the directory containing a header file
    SimpleIncludeDirs(${PKG} ${PKG}_INCLUDE_DIRS ${PKG}_INC_DIR ${HEADERFILE})

    # Look for a fully-qualified library name
    SimpleLibraries(${PKG} ${PKG}_LIBRARIES ${PKG}_LIB_DIR ${LIBRARY})

    if (${PKG}_NOT_FOUND)
      message(STATUS "${PKG} not found.")
    else()
      message(STATUS "Found ${PKG}: ${${PKG}_LIBRARIES}")
    endif()

    # Update the guard variable
    set(CHECK_${PKG}_DONE TRUE CACHE BOOL
      "Is check for ${PKG} components done?")
    mark_as_advanced(CHECK_${PKG}_DONE)
  endif()
endmacro()
