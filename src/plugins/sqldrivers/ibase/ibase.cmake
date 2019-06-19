set(EXTRA_IBASE_LIBS CsCore${BUILD_ABI} CsSql${BUILD_ABI})

set(SQL_PUBLIC_INCLUDES
    ${SQL_PUBLIC_INCLUDES}
    QIBaseDriver
    QIBaseResult
)

set(SQL_INCLUDES
    ${SQL_INCLUDES}
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/ibase/qsql_ibase.h
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/ibase/qibasedriver.h
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/ibase/qibaseresult.h
)

if (FALSE)

set(IBASE_SOURCES
    ${IBASE_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/ibase/qsql_ibase.cpp
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/ibase/main.cpp
)

endif()