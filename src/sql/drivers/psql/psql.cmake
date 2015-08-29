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
    set(EXTRA_PSQL_LDFLAGS
        ${EXTRA_PSQL_LDFLAGS}
        -avoid-version
        -no-undefined
    )

    set(EXTRA_SQL_LIBS
        ${EXTRA_SQL_LIBS}
        ${PostgreSQL_LIBRARIES}
    )

    set(PSQL_SOURCES
        ${PSQL_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/psql/qsql_psql.cpp
        ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/psql/main.cpp
    )

    function_variable_fixup("${EXTRA_PSQL_LDFLAGS}" EXTRA_PSQL_LDFLAGS)

    include_directories(${PostgreSQL_INCLUDE_DIRS})
    add_library(qsqlpsql4 MODULE ${PSQL_SOURCES})
    target_compile_definitions(qsqlpsql4 PRIVATE -DIN_TRUE)
    set_target_properties(qsqlpsql4 PROPERTIES
        LINK_FLAGS ${EXTRA_PSQL_LDFLAGS}
    )

    set(EXTRA_SQL_DRIVERS
        ${EXTRA_SQL_DRIVERS}
        qsqlpsql4
    )

    install(TARGETS qsqlpsql4 DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
