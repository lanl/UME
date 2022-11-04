find_package(Doxygen
  OPTIONAL_COMPONENTS dot)

if(DOXYGEN_FOUND)
  doxygen_add_docs(doxygen
    ${PROJECT_SOURCE_DIR}/src/Ume
    )
endif()
