prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${CMAKE_INSTALL_PREFIX}
libdir=${CMAKE_INSTALL_PREFIX}/lib
includedir=${CMAKE_INSTALL_PREFIX}/include/${PC_NAME}

Name: ${PC_NAME}
Description: ${PC_NAME} Library
Version: ${VERSION}
Libs: ${PC_LIBRARIES}
Cflags: ${PC_CFLAGS} -I${CMAKE_INSTALL_PREFIX}/include -I${CMAKE_INSTALL_PREFIX}/include/${PC_NAME}
Requires: ${PC_REQUIRES}
