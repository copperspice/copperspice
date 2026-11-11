list(APPEND SQL_PUBLIC_INCLUDES
   QDB2Driver
   QDB2Result
)

list(APPEND SQL_INCLUDES
   ${PROJECT_SOURCE_DIR}/src/plugins/sqldrivers/db2/qsql_db2.h
   ${PROJECT_SOURCE_DIR}/src/plugins/sqldrivers/db2/qdb2driver.h
   ${PROJECT_SOURCE_DIR}/src/plugins/sqldrivers/db2/qdb2result.h
)

# if(WITH_DB2_PLUGIN AND DB2_FOUND), unsupported at this time
if (FALSE)

   add_library(CsSqlDb2 MODULE "")
   add_library(CopperSpice::CsSqlDb2 ALIAS CsSqlDb2)

   set_target_properties(CsSqlDb2 PROPERTIES OUTPUT_NAME CsSqlDb2${BUILD_ABI} PREFIX "")

   include_directories(${DB2_INCLUDE_DIRS})

   target_sources(CsSqlDb2
      PRIVATE
      ${PROJECT_SOURCE_DIR}/src/plugins/sqldrivers/db2/main.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/sqldrivers/db2/qsql_db2.cpp
   )

   target_link_libraries(CsSqlDb2
      CsCore
      CsSql
      ${DB2_LIBRARY}
   )

   target_compile_definitions(CsSqlDb2
      PRIVATE
      -DIN_TRUE
      -DQT_PLUGIN
   )

   install(TARGETS CsSqlDb2 DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

