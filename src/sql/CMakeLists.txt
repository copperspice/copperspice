add_library(CsSql SHARED "")
add_library(CopperSpice::CsSql ALIAS CsSql)

set_target_properties(CsSql PROPERTIES OUTPUT_NAME CsSql${BUILD_ABI})

target_compile_definitions(CsSql
   PRIVATE
   -DQT_BUILD_SQL_LIB
   -DQT_SQL_SQLITE
)

set(EXTRA_SQL_CXXFLAGS)
set(EXTRA_SQL_LDFLAGS)

target_include_directories(CsSql
   PUBLIC
   $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include/QtSql>
   $<INSTALL_INTERFACE:include/QtSql>
)

include_directories(
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private
   ${CMAKE_BINARY_DIR}/privateinclude/QtSql/private

   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel
   ${CMAKE_CURRENT_SOURCE_DIR}/models

   ${CMAKE_CURRENT_SOURCE_DIR}/../plugins/sqldrivers/db2
   ${CMAKE_CURRENT_SOURCE_DIR}/../plugins/sqldrivers/ibase
   ${CMAKE_CURRENT_SOURCE_DIR}/../plugins/sqldrivers/mysql
   ${CMAKE_CURRENT_SOURCE_DIR}/../plugins/sqldrivers/oci
   ${CMAKE_CURRENT_SOURCE_DIR}/../plugins/sqldrivers/odbc
   ${CMAKE_CURRENT_SOURCE_DIR}/../plugins/sqldrivers/psql
   ${CMAKE_CURRENT_SOURCE_DIR}/../plugins/sqldrivers/sqlite
)

include(kernel/kernel.cmake)
include(models/models.cmake)

include(../plugins/sqldrivers/db2/db2.cmake)
include(../plugins/sqldrivers/ibase/ibase.cmake)
include(../plugins/sqldrivers/mysql/mysql.cmake)
include(../plugins/sqldrivers/oci/oci.cmake)
include(../plugins/sqldrivers/odbc/odbc.cmake)
include(../plugins/sqldrivers/psql/psql.cmake)
include(../plugins/sqldrivers/sqlite/sqlite.cmake)

macro_generate_public("${SQL_PUBLIC_INCLUDES}"   QtSql)
macro_generate_private("${SQL_PRIVATE_INCLUDES}" QtSql)
macro_generate_misc("${SQL_INCLUDES}" QtSql)

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
   add_definitions(-DENABLE_JSC_MULTIPLE_THREADS=0)

elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
   target_link_libraries(CsSql
      PRIVATE
      ws2_32
   )

endif()

function_variable_fixup("${EXTRA_SQL_CXXFLAGS}" EXTRA_SQL_CXXFLAGS)
function_variable_fixup("${EXTRA_SQL_LDFLAGS}"  EXTRA_SQL_LDFLAGS)

target_sources(CsSql
   PRIVATE
   ${SQL_INCLUDES}
)

target_link_libraries(CsSql
   PRIVATE
   CsCore
)

target_compile_options(CsSql
   PRIVATE
   $<$<COMPILE_LANGUAGE:CXX>:SHELL:${EXTRA_SQL_CXXFLAGS}>
)

set_target_properties(CsSql
   PROPERTIES
   LINK_FLAGS ${EXTRA_SQL_LDFLAGS}
)

function_generate_resources(CsSql)

macro_generate_package(
   CsSql
   QtSql
   "${EXTRA_SQL_CXXFLAGS}"
   "CsCore"
)

install(
   TARGETS CsSql
   EXPORT CopperSpiceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
   RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
   DIRECTORY ${CMAKE_BINARY_DIR}/include/QtSql
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
   COMPONENT Devel
)
