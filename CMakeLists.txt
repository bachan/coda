PROJECT(coda)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules ${CMAKE_MODULE_PATH})

INCLUDE (Compile)
INCLUDE (Package)

USE_PKG (expat)
USE_PKG (Judy)

ADD_STATIC (coda ${LIB_EXPAT} pthread dl)
SET_TARGET_PROPERTIES(coda PROPERTIES VERSION 0.1 SOVERSION 1)

ADD_EXE (lizard coda ${LIB_RDEV} ${LIB_JUDY} ${LIB_EXPAT} pthread dl)
#AUX_SOURCE_DIRECTORY (lizard SRC_LIZARD)
#ADD_EXECUTABLE (lizard ${SRC_LIZARD})
#TARGET_LINK_LIBRARIES (lizard ${LIB_RDEV} ${LIB_JUDY} ${LIB_EXPAT} pthread dl)
#INSTALL (TARGETS lizard DESTINATION bin)
INSTALL (FILES lizard/plugin.hpp DESTINATION include/lizard)

ADD_SHARED (tests/lizard/lz_example)
#ADD_LIBRARY (lz_test SHARED lizard/plugin_test.cpp) # XXX
CONFIGURE_FILE (lizard/lizard.xml.in ${PROJECT_BINARY_DIR}/auto/lizard/lizard.xml)

ENABLE_TESTING()
ADD_EXECUTABLE(test_txml tests/txml/test_txml.cpp)
TARGET_LINK_LIBRARIES(test_txml coda)
ADD_TEST(test_txml test_txml)

