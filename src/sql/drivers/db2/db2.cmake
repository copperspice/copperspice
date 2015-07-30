set(SQL_PUBLIC_INCLUDES
    ${SQL_PUBLIC_INCLUDES}
    QDB2Driver
    QDB2Result
)

set(SQL_INCLUDES
    ${SQL_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/db2/qsql_db2.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/db2/qdb2driver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/db2/qdb2result.h
)

set(SQL_SOURCES
    ${SQL_SOURCES}
    # ${CMAKE_CURRENT_SOURCE_DIR}/drivers/db2/qsql_db2.cpp
)
