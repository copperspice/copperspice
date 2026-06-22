list(APPEND SQL_PUBLIC_INCLUDES
   QMYSQLDriver
   QMYSQLResult
)

list(APPEND SQL_INCLUDES
   ${PROJECT_SOURCE_DIR}/src/plugins/sqldrivers/mysql/qsql_mysql.h
   ${PROJECT_SOURCE_DIR}/src/plugins/sqldrivers/mysql/qmysqldriver.h
   ${PROJECT_SOURCE_DIR}/src/plugins/sqldrivers/mysql/qmysqlresult.h
)

if(WITH_MYSQL_PLUGIN AND MySQL_FOUND)

   add_library(CsSqlMySql MODULE "")
   add_library(CopperSpice::CsSqlMySql ALIAS CsSqlMySql)

   set_target_properties(CsSqlMySql PROPERTIES OUTPUT_NAME CsSqlMySql${BUILD_ABI} PREFIX "")

   include_directories(${MySQL_INCLUDE_DIRS})

   target_sources(CsSqlMySql
      PRIVATE
      ${PROJECT_SOURCE_DIR}/src/plugins/sqldrivers/mysql/qsql_mysql.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/sqldrivers/mysql/main.cpp
    )

   target_link_libraries(CsSqlMySql
      CsCore
      CsSql
      ${MySQL_LIBRARIES}
   )

   if(ZLIB_FOUND)
      target_link_libraries(CsSqlMySql
         ${ZLIB_LIBRARIES}
      )
   endif()

   target_compile_definitions(CsSqlMySql
      PRIVATE
      -DIN_TRUE
      -DQT_PLUGIN
   )

   install(TARGETS CsSqlMySql DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
