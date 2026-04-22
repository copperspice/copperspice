list(APPEND SQL_PUBLIC_INCLUDES
   QSQLiteResult
   QSQLiteDriver
)

list(APPEND SQL_INCLUDES
   ${PROJECT_SOURCE_DIR}/src/plugins/sqldrivers/sqlite/qsql_sqlite.h
   ${PROJECT_SOURCE_DIR}/src/plugins/sqldrivers/sqlite/qsqliteresult.h
   ${PROJECT_SOURCE_DIR}/src/plugins/sqldrivers/sqlite/qsqlitedriver.h
)

if (SQLite3_FOUND)
   target_sources(CsSql
      PRIVATE
      ${PROJECT_SOURCE_DIR}/src/plugins/sqldrivers/sqlite/qsql_sqlite.cpp
   )

   target_link_libraries(CsSql
      PRIVATE
      SQLite::SQLite3
   )

else()
   list(APPEND SQL_PRIVATE_INCLUDES
      ${PROJECT_SOURCE_DIR}/src/3rdparty/sqlite/sqlite3.h
   )

   target_sources(CsSql
      PRIVATE
      ${PROJECT_SOURCE_DIR}/src/3rdparty/sqlite/sqlite3.c
      ${PROJECT_SOURCE_DIR}/src/plugins/sqldrivers/sqlite/qsql_sqlite.cpp
   )

   include_directories(${PROJECT_SOURCE_DIR}/src/3rdparty/sqlite)
endif()
