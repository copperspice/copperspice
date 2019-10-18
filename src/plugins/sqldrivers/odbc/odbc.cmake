list(APPEND SQL_PUBLIC_INCLUDES
    QODBCDriver
    QODBCResult
)

list(APPEND SQL_INCLUDES
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/odbc/qsql_odbc.h
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/odbc/qodbcdriver.h
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/odbc/qodbcresult.h
)

# if(WITH_ODBC_PLUGIN AND ODBC_FOUND), unsupported as this time
if (FALSE)

   add_library(CsSqlOdbc MODULE "")
   add_library(CopperSpice::CsSqlOdbc ALIAS CsSqlOdbc)

   set_target_properties(CsSqlOdbc PROPERTIES OUTPUT_NAME CsSqlOdbc${BUILD_ABI} PREFIX "")

   include_directories(${ODBC_INCLUDE_DIRS})

   target_sources(CsSqlOdbc
      PRIVATE
      ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/odbc/main.cpp
      ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/odbc/qsql_odbc.cpp
   )

   target_link_libraries(CsSqlOdbc
      CsCore
      CsSql
      ${ODBC_LIBRARY}
   )

   target_compile_definitions(CsSqlOdbc
      PRIVATE
      -DIN_TRUE
      -DQT_PLUGIN
   )

   install(TARGETS CsSqlOdbc DESTINATION ${CMAKE_INSTALL_LIBDIR})

endif()
