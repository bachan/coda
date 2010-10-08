PROJECT (coda)
CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
INCLUDE (CMakeModules/Compile.cmake)
INCLUDE (CMakeModules/Package.cmake)

USE_PACKAGE (expat expat.h)
USE_PACKAGE (Judy  Judy.h)

FIND_PATH(INCLUDE_MYSQL mysql.h PATHS /usr/include/mysql /usr/local/include/mysql)
INCLUDE_DIRECTORIES(${INCLUDE_MYSQL})

SET(MYSQL_NAMES mysqlclient mysqlclient_r)
FIND_LIBRARY(MYSQL_LIBRARY
  NAMES ${MYSQL_NAMES}
  PATHS /usr/lib /usr/local/lib /usr/lib64 /usr/local/lib64
  PATH_SUFFIXES mysql
)

MAKE_STATIC (coda ${LIB_expat} pthread dl)
SET_TARGET_PROPERTIES(coda PROPERTIES VERSION 0.1 SOVERSION 1)

MAKE_PROGRAM (blizzard coda ${LIB_Judy} ${LIB_expat} pthread dl)
INSTALL (FILES blizzard/plugin.hpp DESTINATION include/blizzard)
INSTALL_TEMPLATE (blizzard/blizzard.xml.in DESTINATION etc/blizzard)
MAKE_SHARED (tests/blizzard/blzmod_example)

ENABLE_TESTING()
MAKE_TEST (tests/txml coda)

