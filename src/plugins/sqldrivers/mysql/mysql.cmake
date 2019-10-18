list(APPEND SQL_PUBLIC_INCLUDES
   QMYSQLDriver
   QMYSQLResult
)

list(APPEND SQL_INCLUDES
   ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/mysql/qsql_mysql.h
   ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/mysql/qmysqldriver.h
   ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/mysql/qmysqlresult.h
)

if(WITH_MYSQL_PLUGIN AND MYSQL_FOUND)

   add_library(CsSqlMySql MODULE "")
   add_library(CopperSpice::CsSqlMySql ALIAS CsSqlMySql)

   set_target_properties(CsSqlMySql PROPERTIES OUTPUT_NAME CsSqlMySql${BUILD_ABI} PREFIX "")

   include_directories(${MYSQL_INCLUDES})

   target_sources(CsSqlMySql
      PRIVATE
      ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/mysql/qsql_mysql.cpp
      ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/mysql/main.cpp
    )

   target_link_libraries(CsSqlMySql
      CsCore
      CsSql
      ${MYSQL_LIBRARIES}
      ${ZLIB_LIBRARIES}
   )

   target_compile_definitions(CsSqlMySql
      PRIVATE
      -DIN_TRUE
      -DQT_PLUGIN
   )

   install(TARGETS CsSqlMySql DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
