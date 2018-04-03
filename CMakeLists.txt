PROJECT (coda)
CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
INCLUDE (CMakeMacro.cmake)

USE_PACKAGE (expat expat.h)

AUX_SOURCE_DIRECTORY (coda/ SRC_coda)
ADD_LIBRARY (coda ${SRC_coda})
TARGET_LINK_LIBRARIES (coda ${LIB_expat} pthread)
INSTALL (TARGETS coda DESTINATION lib)
INSTALL (DIRECTORY coda/ DESTINATION include/coda FILES_MATCHING PATTERN "*.h")
INSTALL (DIRECTORY coda/ DESTINATION include/coda FILES_MATCHING PATTERN "*.hpp")
INSTALL (DIRECTORY coda/ DESTINATION include/coda FILES_MATCHING PATTERN "*.tcc")
SET_TARGET_PROPERTIES (coda PROPERTIES VERSION 0.1 SOVERSION 1)

ADD_EXECUTABLE (coda_url_parser tests/url_parser/test.cpp)
TARGET_LINK_LIBRARIES (coda_url_parser coda)
INSTALL (TARGETS coda_url_parser DESTINATION bin)

ADD_EXECUTABLE (test_cache tests/cache/test.cpp)
TARGET_LINK_LIBRARIES (test_cache coda)

ADD_EXECUTABLE (test_stringstorage tests/stringstorage/test.c)
TARGET_LINK_LIBRARIES (test_stringstorage coda)

