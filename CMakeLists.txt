PROJECT (coda)
CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
INCLUDE (CMakeModules/Compile.cmake)
INCLUDE (CMakeModules/Package.cmake)

USE_PACKAGE (expat expat.h)

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

#MAKE_STATIC (mysqlslave ${MYSQL_LIBRARY})

