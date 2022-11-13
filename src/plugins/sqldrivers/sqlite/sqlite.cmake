list(APPEND SQL_PUBLIC_INCLUDES
   QSQLiteResult
   QSQLiteDriver
)

list(APPEND SQL_INCLUDES
   ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/sqlite/qsql_sqlite.h
   ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/sqlite/qsqliteresult.h
   ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/sqlite/qsqlitedriver.h
)

if (SQLite3_FOUND)
   target_sources(CsSql
      PRIVATE
      ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/sqlite/qsql_sqlite.cpp
   )

   target_link_libraries(CsSql
      PRIVATE
      SQLite::SQLite3
   )

else()
   list(APPEND SQL_PRIVATE_INCLUDES
      ${CMAKE_SOURCE_DIR}/src/3rdparty/sqlite/sqlite3.h
   )

   target_sources(CsSql
      PRIVATE
      ${CMAKE_SOURCE_DIR}/src/3rdparty/sqlite/sqlite3.c
      ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/sqlite/qsql_sqlite.cpp
   )

   include_directories(${CMAKE_SOURCE_DIR}/src/3rdparty/sqlite)
endif()
