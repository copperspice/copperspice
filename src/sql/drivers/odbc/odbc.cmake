set(SQL_PUBLIC_INCLUDES
    ${SQL_PUBLIC_INCLUDES}
    QODBCDriver
    QODBCResult
)

set(SQL_INCLUDES
    ${SQL_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/odbc/qsql_odbc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/odbc/qodbcdriver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/odbc/qodbcresult.h
)

set(SQL_SOURCES
    ${SQL_SOURCES}
    # ${CMAKE_CURRENT_SOURCE_DIR}/drivers/odbc/qsql_odbc.cpp
)
