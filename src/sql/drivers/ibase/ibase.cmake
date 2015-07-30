set(SQL_PUBLIC_INCLUDES
    ${SQL_PUBLIC_INCLUDES}
    QIBaseDriver
    QIBaseResult
)

set(SQL_INCLUDES
    ${SQL_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/ibase/qsql_ibase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/ibase/qibasedriver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/ibase/qibaseresult.h
)

set(SQL_SOURCES
    ${SQL_SOURCES}
    # ${CMAKE_CURRENT_SOURCE_DIR}/drivers/ibase/qsql_ibase.cpp
)
