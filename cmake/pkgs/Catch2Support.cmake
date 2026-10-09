option(USE_CATCH2 "Enable support for Catch2 testing" ON)

if (USE_CATCH2)
  find_package(Catch2 3 REQUIRED)
  include(CTest)
  include(Catch)
endif()

# Note: add_subdirectory(test) is deliberately *not* here.  A pkgs/ module
# runs before the other pkgs/ modules and before add_subdirectory(src), so a
# test directory added from one sees neither the variables the later modules
# set nor the targets src/ defines -- on a fresh configure.  On a reconfigure
# it reads them back from the cache and sees them, which is worse than not
# seeing them at all.  The top-level CMakeLists.txt adds test/ after src/.
