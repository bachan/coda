#IF (CMAKE_COMPILER_IS_GNUCC)
# SET (FLAGS_DEFAULT  "-fPIC -pipe")
# SET (FLAGS_WARNING  "-Wall -Werror -Wno-long-long -Wno-variadic-macros -Wno-strict-aliasing")# -Wextra -pedantic")
# SET (FLAGS_CXX_LANG "-Wno-deprecated")
# SET (FLAGS_RELEASE  "-O3 -march=native -fomit-frame-pointer -funroll-loops -DNDEBUG")
# SET (FLAGS_DEBUG    "-ggdb")

# TODO
# -pedantic: stupid gcc-4.4 warning about empty macro arguments
# -fno-strict-aliasing: removes following optimizations
# -Wno-strict-aliasing: removes warning

# SET (CMAKE_C_FLAGS_DEBUG     "${FLAGS_DEFAULT} ${FLAGS_WARNING} ${FLAGS_DEBUG}")
# SET (CMAKE_C_FLAGS_RELEASE   "${FLAGS_DEFAULT} ${FLAGS_WARNING} ${FLAGS_RELEASE}")

# SET (CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   ${FLAGS_CXX_LANG}")
# SET (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${FLAGS_CXX_LANG}")

# Debug options to prevent buffer overflows, see:
# http://gcc.gnu.org/ml/gcc-patches/2004-09/msg02055.html
# ADD_DEFINITIONS (-D_FORTIFY_SOURCE=2)
# ADD_DEFINITIONS (-fstack-protector)
#ENDIF (CMAKE_COMPILER_IS_GNUCC)

IF (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  SET (CMAKE_BUILD_TYPE RELEASE)
  SET (CMAKE_BUILD_TYPE RELEASE CACHE STRING "Build type" FORCE)
ENDIF (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)

