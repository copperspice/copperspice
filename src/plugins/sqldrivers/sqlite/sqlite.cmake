set(SQL_PUBLIC_INCLUDES
    ${SQL_PUBLIC_INCLUDES}
    QSQLiteResult
    QSQLiteDriver
)

set(SQL_INCLUDES
    ${SQL_INCLUDES}
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/sqlite/qsql_sqlite.h
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/sqlite/qsqliteresult.h
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/sqlite/qsqlitedriver.h
)

set(SQL_PRIVATE_INCLUDES
    ${SQL_PRIVATE_INCLUDES}
    ${CMAKE_SOURCE_DIR}/src/3rdparty/sqlite/sqlite3.h
)

set(SQL_SOURCES
 ${SQL_SOURCES}
 ${CMAKE_SOURCE_DIR}/src/3rdparty/sqlite/sqlite3.c
 ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/sqlite/qsql_sqlite.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/src/3rdparty/sqlite)

