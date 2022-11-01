
option(USE_OPENACC "Enable support for OpenACC code")

if(USE_OPENACC)
  find_package(OpenACC REQUIRED)
  set(OPENACC_LINK_LIBRARIES OpenACC::OpenACC_CXX)
endif()
