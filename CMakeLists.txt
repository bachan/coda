PROJECT (coda)
CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
INCLUDE (CMakeModules/Compile.cmake)
INCLUDE (CMakeModules/Package.cmake)

USE_PACKAGE (expat expat.h)
USE_PACKAGE (Judy  Judy.h)

MAKE_STATIC (coda ${LIB_expat} pthread dl)
SET_TARGET_PROPERTIES(coda PROPERTIES VERSION 0.1 SOVERSION 1)

MAKE_PROGRAM (blizzard coda ${LIB_Judy} ${LIB_expat} pthread dl)
INSTALL (FILES blizzard/plugin.hpp DESTINATION include/blizzard)

MAKE_SHARED (tests/blizzard/blzmod_example)
#ADD_LIBRARY (lz_test SHARED lizard/plugin_test.cpp) # XXX
CONFIGURE_FILE (blizzard/blizzard.xml.in ${PROJECT_BINARY_DIR}/auto/blizzard/blizzard.xml)

ENABLE_TESTING()
ADD_EXECUTABLE(test_txml tests/txml/test_txml.cpp)
TARGET_LINK_LIBRARIES(test_txml coda)
ADD_TEST(test_txml test_txml)

