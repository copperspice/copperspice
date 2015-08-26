set(SQL_PUBLIC_INCLUDES
    ${SQL_PUBLIC_INCLUDES}
    QPSQLDriver
    QPSQLResult
)

set(SQL_INCLUDES
    ${SQL_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/psql/qsql_psql.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/psql/qpsqldriver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/psql/qpsqlresult.h
)

if(WITH_PSQL_PLUGIN AND PostgreSQL_FOUND)
    # TODO: those should probably apply to a single object
    set(EXTRA_SQL_LDFLAGS
        ${EXTRA_SQL_LDFLAGS}
        -avoid-version
        -no-undefined
        # -module
    )

    set(EXTRA_SQL_LIBS
        ${EXTRA_SQL_LIBS}
        ${PostgreSQL_LIBRARIES}
    )

    set(SQL_SOURCES
        ${SQL_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/psql/qsql_psql.cpp
        ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/psql/main.cpp
    )

    include_directories(${PostgreSQL_INCLUDE_DIRS})
    add_definitions(-DIN_TRUE)
endif()
